#include "Octree.hpp"
#include "OctreeInput.hpp"
#include "OctreeResourceCallbacks.hpp"
#include <new>
#include "../diagnostics/DebugOutput.hpp"
#include <string.h>

namespace
{
    // The .8 INFO chunk contains the original x86 fields beginning at
    // OctreeNode::bounds. Child slots are serialized as 32-bit presence
    // indicators, NOT as native pointer-sized OctreeNode* values.
    struct DiskOctreeNodeInfo
    {
        float bounds[6];
        DWORD vertexBuffer;
        DWORD flags;
        DWORD children[8];
    };
    static_assert(sizeof(DiskOctreeNodeInfo) == 64);
    static_assert(offsetof(DiskOctreeNodeInfo, children) == 32);

    void ApplyNodeInfo(OctreeNode& node, const DiskOctreeNodeInfo& disk, bool branch)
    {
        memcpy(node.bounds, disk.bounds, sizeof(disk.bounds));
        node.vertexBuffer = disk.vertexBuffer;
        node.flags = disk.flags;
        for (DWORD index = 0; index < 8; ++index)
        {
            if (branch && disk.children[index])
            {
                node.children[index] = new OctreeNode;
                node.children[index]->parent = &node;
            }
            else
            {
                node.children[index] = nullptr;
            }
        }
    }

    // Scan the same sequential stream as the original; container extents are not guards.
    bool FindChunk(OctreeInput& stream, DWORD tag, DWORD& remaining)
    {
        DWORD header[2];
        for (;;)
        {
            if (remaining)
            {
                OctreeSeek(stream, remaining, SEEK_CUR);
            }
            if (OctreeRead(header, 4, 2, stream) != 2)
            {
                return false;
            }
            remaining = header[1];
            if (header[0] == tag)
            {
                return true;
            }
        }
    }
} // namespace

int OCTREE::Read8File(FILE* stream, DWORD attributes, OCTREE** output)
{
    if (!stream)
    {
        return 2;
    }
    OctreeFileInput input(stream);
    return Read8Input(input, attributes, output);
}

int OCTREE::Read8Input(OctreeInput& stream, DWORD attributes, OCTREE** output)
{
    OCTREE* tree = new OCTREE(attributes);
    DWORD header[2];
    DWORD remaining = 0;
    DWORD totalNames;
    if (OctreeRead(header, 4, 2, stream) != 2 || header[0] != 0x45525438)
    {
        goto Failure;
    }
    if (!FindChunk(stream, 0x44414548, remaining))
    {
        goto Failure; // HEAD.
    }
    remaining -= static_cast<DWORD>(OctreeRead(&tree->version, 1, remaining < 16 ? remaining : 16, stream));
    if (tree->flags & 4)
    {
        if (!FindChunk(stream, 0x4454474C, remaining))
        {
            goto Failure; // LGTD.
        }
        if (OctreeRead(header, 4, 2, stream) != 2 || header[0] != 0x4F464E49)
        {
            goto Failure;
        }
        OctreeRead(&tree->lightCount, 4, 1, stream);
        tree->lights = new OctreeLight[tree->lightCount];
        for (DWORD index = 0; index < tree->lightCount; ++index)
        {
            if (OctreeRead(header, 4, 2, stream) != 2 || header[0] != 0x5254474C)
            {
                goto Failure;
            }
            OctreeRead(tree->lights + index, 1, header[1], stream);
            ++tree->lights[index].words[1];
        }
        remaining = 0;
    }
    else
    {
        tree->lightCount = 0;
        tree->lights = NULL;
        JE::LogDebugString("\n\n*** [OCTREE]: THIS OCTREE DOES NOT CONTAIN ANY (STATIC) LIGHTS!!!\n\n");
    }
    if (tree->flags & 8)
    {
        tree->lightGrid = new OctreeLightGrid;
        memset(tree->lightGrid, 0, sizeof(OctreeLightGrid));
        if (!FindChunk(stream, 0x444C564C, remaining))
        {
            goto Failure; // LVLD.
        }
        if (OctreeRead(header, 4, 2, stream) != 2 || header[0] != 0x4F464E49)
        {
            goto Failure;
        }
        remaining = header[1] - static_cast<DWORD>(OctreeRead(tree->lightGrid->words, 1, 16, stream));
        OctreeSeek(stream, remaining, SEEK_CUR);
        if (OctreeRead(header, 4, 2, stream) != 2 || header[0] != 0x304C564C)
        {
            goto Failure;
        }
        tree->lightGrid->data = new BYTE[header[1]];
        OctreeRead(tree->lightGrid->data, 1, header[1], stream);
        remaining = 0;
    }
    else
    {
        JE::LogDebugString("\n\n*** [OCTREE]: THIS OCTREE DOES NOT CONTAIN STATIC LIGHTING DATA!!!\n\n");
    }
    if (!FindChunk(stream, 0x4454414D, remaining))
    {
        goto Failure; // MATD.
    }
    remaining = 0;
    if (!FindChunk(stream, 0x4F464E49, remaining))
    {
        goto Failure; // INFO.
    }
    OctreeRead(&tree->materialCount, 4, 1, stream);
    tree->materials = new OctreeMaterial[tree->materialCount];
    memset(tree->materials, 0, 196 * tree->materialCount);
    for (DWORD index = 0; index < tree->materialCount; ++index)
    {
        if (OctreeRead(header, 4, 2, stream) != 2 || header[0] != 0x5254414D)
        {
            goto Failure;
        }
        DWORD read =
            static_cast<DWORD>(OctreeRead(tree->materials + index, 1, header[1] < 196 ? header[1] : 196, stream));
        if (header[1] - read)
        {
            OctreeSeek(stream, header[1] - read, SEEK_CUR);
        }
    }
    remaining = 0;
    if (!FindChunk(stream, 0x44545854, remaining))
    {
        goto Failure; // TXTD.
    }
    OctreeRead(&tree->firstLightmap, 4, 1, stream);
    OctreeRead(&totalNames, 4, 1, stream);
    remaining -= 8;
    tree->lightmapCount = totalNames - tree->firstLightmap + 1;
    tree->textureCount = totalNames;
    tree->materialValues = new DWORD[tree->materialCount];
    tree->textureHandles = new DWORD[tree->firstLightmap];
    tree->lightmapHandles = new DWORD[tree->lightmapCount];
    tree->textureNames = new char*[tree->textureCount];
    tree->textureNames[0] = NULL;
    for (DWORD index = 1; index < tree->textureCount; ++index)
    {
        if (OctreeRead(header, 4, 2, stream) != 2)
        {
            goto Failure;
        }
        tree->textureNames[index] = new char[header[1]];
        remaining -= 8 + static_cast<DWORD>(OctreeRead(tree->textureNames[index], 1, header[1], stream));
    }
    if (!FindChunk(stream, 0x44585456, remaining))
    {
        goto Failure; // VTXD.
    }
    OctreeRead(&tree->vertexBufferCount, 4, 1, stream);
    tree->vertexBuffers = new OctreeVertexStreams[tree->vertexBufferCount];
    for (DWORD index = 0; index < tree->vertexBufferCount; ++index)
    {
        tree->vertexBuffers[index].ReadVertexInput(stream, static_cast<WORD>(attributes));
    }
    remaining = 0;
    if (!FindChunk(stream, 0x45455254, remaining))
    {
        goto Failure; // TREE size is ignored.
    }
    tree->indexedNodeCounts = new DWORD[tree->vertexBufferCount];
    tree->ReadTreeInput(stream); // The original deliberately ignores both child statuses.
    tree->sceneNodeCounts = new DWORD[tree->vertexBufferCount];
    tree->traversal = new OctreeTraversal[tree->depth];
    for (DWORD index = 0; index < tree->depth; ++index)
    {
        memset(tree->traversal + index, 0, sizeof(OctreeTraversal));
    }
    tree->visibleTreeNodes = NULL;
    tree->visibleBuckets = NULL;
    // Original x86 layout used 1,025 four-byte slots. Store actual native pointers
    // and initialize every slot, including the trailing null terminator.
    tree->scratch = new OctreeSpatialObject*[tree->handles.capacity + 1]{};
    *output = tree;
    return 0;

Failure:
    // Partial-owner fields remain unwritten, as in the original. Malformed input
    // and allocation failure are outside the current supported/validated domain.
    tree->Release();
    return 1;
}

int OCTREE::ReadTree(FILE* stream)
{
    if (!depth)
    {
        return 1;
    }
    DWORD* childCounters = new DWORD[depth];
    for (DWORD index = 0; index < depth; ++index)
    {
        childCounters[index] = 0;
    }
    for (DWORD index = 0; index < vertexBufferCount; ++index)
    {
        indexedNodeCounts[index] = 0;
    }
    OctreeNode* node = &root;
    node->parent = NULL;
    DWORD header[2] = { 0, 0 };
    DWORD* currentCounter = childCounters;
    do
    {
        DWORD nextChild = *currentCounter;
        if (!nextChild)
        {
            if (header[0] != 0x45444F4E && header[0] != 0x4641454C)
            {
                if (g_OctreeReadCallback(header, 4, 2, stream) != 2)
                {
                    goto Failure;
                }
                if (header[0] != 0x45444F4E && header[0] != 0x4641454C)
                {
                    goto Failure;
                }
            }
            DWORD kind = header[0];
            header[1] = 0;
            if (kind == 0x45444F4E) // NODE.
            {
                for (;;)
                {
                    if (header[1])
                    {
                        g_OctreeSeekCallback(stream, header[1], SEEK_CUR);
                    }
                    if (g_OctreeReadCallback(header, 4, 2, stream) != 2)
                    {
                        goto Failure;
                    }
                    if (header[0] == 0x4F464E49)
                    {
                        break; // INFO.
                    }
                }
                // Read the on-disk x86 layout into a 64-byte record. Reading
                // directly into OctreeNode overwrites x64 pointer fields.
                if (header[1] < sizeof(DiskOctreeNodeInfo))
                {
                    goto Failure;
                }
                DiskOctreeNodeInfo disk{};
                if (g_OctreeReadCallback(&disk, 1, sizeof(disk), stream) != sizeof(disk))
                {
                    goto Failure;
                }
                header[1] -= sizeof(disk);
                ApplyNodeInfo(*node, disk, true);
                for (;;)
                {
                    if (header[1])
                    {
                        g_OctreeSeekCallback(stream, header[1], SEEK_CUR);
                    }
                    if (g_OctreeReadCallback(header, 4, 2, stream) != 2)
                    {
                        goto Failure;
                    }
                    if (header[0] == 0x47554244) // DBUG.
                    {
                        g_OctreeSeekCallback(stream, header[1], SEEK_CUR);
                        header[1] = 0;
                    }
                    if (header[0] == 0x45444F4E || header[0] == 0x4641454C || header[0] == 0x44584449 ||
                        header[0] == 0x53474C46)
                    {
                        break;
                    }
                }
                if (header[0] == 0x44584449) // IDXD.
                {
                    if (g_OctreeReadCallback(header, 4, 2, stream) != 2 || header[0] != 0x53584449)
                    {
                        goto Failure;
                    }
                    ++indexedNodeCounts[node->vertexBuffer];
                    node->wordCount = header[1] >> 1;
                    node->indices = new WORD[node->wordCount];
                    g_OctreeReadCallback(node->indices, 1, header[1], stream);
                    // Branch nodes deliberately ignore this read's status.
                    g_OctreeReadCallback(header, 4, 2, stream);
                    if (header[0] == 0x53474C46) // FLGS count is in DWORDs.
                    {
                        node->triangleFlags = new DWORD[header[1]];
                        g_OctreeReadCallback(node->triangleFlags, 4, header[1], stream);
                    }
                }
                if (!node->indices)
                {
                    node->flags |= 0x80000000;
                }
            }
            else // LEAF has its own required IDXS/FLGS read policy.
            {
                for (;;)
                {
                    if (header[1])
                    {
                        g_OctreeSeekCallback(stream, header[1], SEEK_CUR);
                    }
                    if (g_OctreeReadCallback(header, 4, 2, stream) != 2)
                    {
                        goto Failure;
                    }
                    if (header[0] == 0x4F464E49)
                    {
                        break;
                    }
                }
                if (header[1] < offsetof(DiskOctreeNodeInfo, children))
                {
                    goto Failure;
                }
                DiskOctreeNodeInfo disk{};
                const DWORD bytesToRead = header[1] < sizeof(disk) ? header[1] : sizeof(disk);
                if (g_OctreeReadCallback(&disk, 1, bytesToRead, stream) != bytesToRead)
                {
                    goto Failure;
                }
                header[1] -= bytesToRead;
                ApplyNodeInfo(*node, disk, false);
                for (;;)
                {
                    if (header[1])
                    {
                        g_OctreeSeekCallback(stream, header[1], SEEK_CUR);
                    }
                    if (g_OctreeReadCallback(header, 4, 2, stream) != 2)
                    {
                        goto Failure;
                    }
                    if (header[0] == 0x47554244)
                    {
                        g_OctreeSeekCallback(stream, header[1], SEEK_CUR);
                        header[1] = 0;
                    }
                    if (header[0] == 0x45444F4E || header[0] == 0x4641454C)
                    {
                        goto Failure;
                    }
                    if (header[0] == 0x44584449)
                    {
                        break;
                    }
                }
                if (g_OctreeReadCallback(header, 4, 2, stream) != 2 || header[0] != 0x53584449)
                {
                    goto Failure;
                }
                ++indexedNodeCounts[node->vertexBuffer];
                node->wordCount = header[1] >> 1;
                node->indices = new WORD[node->wordCount];
                g_OctreeReadCallback(node->indices, 1, header[1], stream);
                if (g_OctreeReadCallback(header, 4, 2, stream) != 2 || header[0] != 0x53474C46)
                {
                    goto Failure;
                }
                node->triangleFlags = new DWORD[header[1]];
                g_OctreeReadCallback(node->triangleFlags, 4, header[1], stream);
            }
        }
        {
            while (nextChild < 8)
            {
                OctreeNode* child = node->children[nextChild++];
                if (child)
                {
                    if (currentCounter + 1 == childCounters + depth)
                    {
                        goto Failure;
                    }
                    *currentCounter = nextChild;
                    node = child;
                    ++currentCounter;
                    goto NextNode;
                }
            }
        }
        *currentCounter = 0;
        node = node->parent;
        --currentCounter;
    NextNode:;
    } while (node);
    delete[] childCounters;
    return 0;
Failure:
    if (childCounters)
    {
        delete[] childCounters;
    }
    return 1;
}

int OCTREE::ReadTreeInput(OctreeInput& stream)
{
    if (!depth)
    {
        return 1;
    }
    DWORD* childCounters = new DWORD[depth]{};
    for (DWORD index = 0; index < vertexBufferCount; ++index)
    {
        indexedNodeCounts[index] = 0;
    }
    OctreeNode* node = &root;
    node->parent = NULL;
    DWORD header[2] = { 0, 0 };
    int level = 0;
    do
    {
        if (!childCounters[level])
        {
            if (header[0] != 0x45444F4E && header[0] != 0x4641454C)
            {
                if (OctreeRead(header, 4, 2, stream) != 2)
                {
                    goto Failure;
                }
                if (header[0] != 0x45444F4E && header[0] != 0x4641454C)
                {
                    goto Failure;
                }
            }
            bool branch = header[0] == 0x45444F4E;
            DWORD remaining = 0;
            if (!FindChunk(stream, 0x4F464E49, remaining))
            {
                goto Failure;
            }
            // INFO contains 32-bit serialized child indicators. It must
            // never be read directly into a pointer-bearing OctreeNode.
            if (remaining < offsetof(DiskOctreeNodeInfo, children) ||
                (branch && remaining < sizeof(DiskOctreeNodeInfo)))
            {
                goto Failure;
            }
            DiskOctreeNodeInfo disk{};
            const DWORD bytesToRead = remaining < sizeof(disk) ? remaining : sizeof(disk);
            if (OctreeRead(&disk, 1, bytesToRead, stream) != bytesToRead)
            {
                goto Failure;
            }
            remaining -= bytesToRead;
            ApplyNodeInfo(*node, disk, branch);
            for (;;)
            {
                if (remaining)
                {
                    OctreeSeek(stream, remaining, SEEK_CUR);
                }
                if (OctreeRead(header, 4, 2, stream) != 2)
                {
                    goto Failure;
                }
                remaining = header[1];
                if (header[0] == 0x47554244) // DBUG.
                {
                    OctreeSeek(stream, remaining, SEEK_CUR);
                    remaining = 0;
                }
                if (header[0] == 0x45444F4E || header[0] == 0x4641454C)
                {
                    if (!branch)
                    {
                        goto Failure;
                    }
                    break;
                }
                if (branch && header[0] == 0x53474C46)
                {
                    break;
                }
                if (header[0] == 0x44584449) // IDXD, followed by IDXS.
                {
                    if (OctreeRead(header, 4, 2, stream) != 2 || header[0] != 0x53584449)
                    {
                        goto Failure;
                    }
                    ++indexedNodeCounts[node->vertexBuffer];
                    node->wordCount = header[1] >> 1;
                    node->indices = new WORD[node->wordCount];
                    OctreeRead(node->indices, 1, header[1], stream);
                    size_t read = OctreeRead(header, 4, 2, stream);
                    if (!branch && (read != 2 || header[0] != 0x53474C46))
                    {
                        goto Failure;
                    }
                    if (header[0] == 0x53474C46)
                    {
                        // FLGS length is a DWORD count, unlike ordinary byte-sized chunks.
                        node->triangleFlags = new DWORD[header[1]];
                        OctreeRead(node->triangleFlags, 4, header[1], stream);
                    }
                    break;
                }
            }
            if (branch && !node->indices)
            {
                node->flags |= 0x80000000;
            }
        }
        {
            DWORD nextChild = childCounters[level];
            while (nextChild < 8)
            {
                OctreeNode* child = node->children[nextChild++];
                if (child)
                {
                    if (static_cast<DWORD>(level + 1) >= depth)
                    {
                        goto Failure;
                    }
                    childCounters[level] = nextChild;
                    node = child;
                    ++level;
                    goto NextNode;
                }
            }
        }
        childCounters[level] = 0;
        node = node->parent;
        --level;
    NextNode:;
    } while (node);
    delete[] childCounters;
    return 0;

Failure:
    delete[] childCounters;
    return 1;
}
