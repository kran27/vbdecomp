#include "ResourceIndex.hpp"
#include <cstring>
#include <cstddef>

namespace JE { namespace Resources {

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
typedef char ResourceIndexBytes[(sizeof(ResourceIndex) == 20) ? 1 : -1];
typedef char ResourceBucketBytes[(sizeof(std::list<ResourceDescriptor>) == 12) ? 1 : -1];
typedef char ResourceDescriptorBytes[(sizeof(ResourceDescriptor) == 72) ? 1 : -1];
typedef char ResourceNameOffset[(offsetof(ResourceDescriptor, name) == 12) ? 1 : -1];
typedef char ResourcePathOffset[(offsetof(ResourceDescriptor, path) == 40) ? 1 : -1];
typedef char ResourceCachedOffset[(offsetof(ResourceDescriptor, cached) == 68) ? 1 : -1];
#endif

ResourceIndex::ResourceIndex() : bucketCount(0) {} //50C980, actual default owner.

// The adjacent native prime runs support this260-element extent; authorial bounds remain inferred.
namespace {
const Net::UInt32 resourceHashWeights[260] = {
    2u, 3u, 5u, 7u, 11u, 13u, 17u, 19u, 23u, 29u,
    31u, 37u, 41u, 43u, 47u, 53u, 59u, 61u, 67u, 71u,
    73u, 79u, 83u, 89u, 97u, 101u, 103u, 107u, 109u, 113u,
    127u, 131u, 137u, 139u, 149u, 151u, 157u, 163u, 167u, 173u,
    179u, 181u, 191u, 193u, 197u, 199u, 211u, 223u, 227u, 229u,
    233u, 239u, 241u, 251u, 257u, 263u, 269u, 271u, 277u, 281u,
    283u, 293u, 307u, 311u, 313u, 317u, 331u, 337u, 347u, 349u,
    353u, 359u, 367u, 373u, 379u, 383u, 389u, 397u, 401u, 409u,
    419u, 421u, 431u, 433u, 439u, 443u, 449u, 457u, 461u, 463u,
    467u, 479u, 487u, 491u, 499u, 503u, 509u, 521u, 523u, 541u,
    547u, 557u, 563u, 569u, 571u, 577u, 587u, 593u, 599u, 601u,
    607u, 613u, 617u, 619u, 631u, 641u, 643u, 647u, 653u, 659u,
    661u, 673u, 677u, 683u, 691u, 701u, 709u, 719u, 727u, 733u,
    739u, 743u, 751u, 757u, 761u, 769u, 773u, 787u, 797u, 809u,
    811u, 821u, 823u, 827u, 829u, 839u, 853u, 857u, 859u, 863u,
    877u, 881u, 883u, 887u, 907u, 911u, 919u, 929u, 937u, 941u,
    947u, 953u, 967u, 971u, 977u, 983u, 991u, 997u, 1009u, 1013u,
    1019u, 1021u, 1031u, 1033u, 1039u, 1049u, 1051u, 1061u, 1063u, 1069u,
    1087u, 1091u, 1093u, 1097u, 1103u, 1109u, 1117u, 1123u, 1129u, 1151u,
    1153u, 1163u, 1171u, 1181u, 1187u, 1193u, 1201u, 1213u, 1217u, 1223u,
    1229u, 1231u, 1237u, 1249u, 1259u, 1277u, 1279u, 1283u, 1289u, 1291u,
    1297u, 1301u, 1303u, 1307u, 1319u, 1321u, 1327u, 1361u, 1367u, 1373u,
    1381u, 1399u, 1409u, 1423u, 1427u, 1429u, 1433u, 1439u, 1447u, 1451u,
    1453u, 1459u, 1471u, 1481u, 1483u, 1487u, 1489u, 1493u, 1499u, 1511u,
    1523u, 1531u, 1543u, 1549u, 1553u, 1559u, 1567u, 1571u, 1579u, 1583u,
    1597u, 1601u, 1607u, 1609u, 1613u, 1619u, 1621u, 1627u, 1637u, 1657u
};
}
Net::UInt32 ResourceIndex::Hash(const char* name, Net::UInt32 type) const
{
    Net::UInt32 result=0, sum=0, product=1;
    int length=0;
    if (*name)
    {
        do ++length; while (name[length]);
        if (length>0)
        {
            for (int i=0;i<length;++i)
            {
                Net::UInt32 character=name[i]; //VC7 default signedchar; native MOVSX
                if (character>='a' && character<='z') character-=32;
                Net::UInt32 term=resourceHashWeights[i]*(character&0x2Fu);
                sum+=term;
                product*=term%bucketCount;
            }
            result=(type+product+sum)%bucketCount;
        }
    }
    return result;
}
void ResourceIndex::Initialize(Net::UInt32 count)
{
    buckets.resize(count);
    bucketCount=count; //only after resize returns, as50C9EE
}


ResourceDescriptor* ResourceIndex::Find(const char* name, Net::UInt32 type)
{
    const Net::UInt32 bucket = Hash(name, type);
    Bucket::iterator item = buckets[bucket].begin();
    for (; item != buckets[bucket].end(); ++item)
        if (!_stricmp(item->name.c_str(), name) && item->type == type)
            return &*item;
    return NULL;
}

void ResourceIndex::Remove(const char* name, Net::UInt32 type)
{
    const Net::UInt32 bucket = Hash(name, type);
    Bucket::iterator item = buckets[bucket].begin();
    for (; item != buckets[bucket].end(); ++item)
        if (!_stricmp(item->name.c_str(), name) && item->type == type)
        {
            buckets[bucket].erase(item);
            return;
        }
}

void ResourceIndex::Upsert(const char* name, Net::UInt32 type,
    Net::UInt32 flags, const char* path, Net::UInt32 ordinal)
{
    ResourceDescriptor descriptor; //both empty strings construct before Hash/lookup
    const Net::UInt32 bucket=Hash(name,type);
    std::list<ResourceDescriptor>::iterator item=buckets[bucket].begin();
    ResourceDescriptor* found=NULL;
    for (;item!=buckets[bucket].end();++item) //reread vector begin/head after callbacks
    {
        if (!_stricmp(item->name.c_str(),name) && item->type==type)
        {
            found=&*item;
            break;
        }
    }
    if (!found)
    {
        descriptor.name.assign(name);
        descriptor.type=type;
        descriptor.flags=flags;
        descriptor.path.assign(path);
        descriptor.ordinal=ordinal;
        buckets[bucket].push_back(descriptor); //implicit complete copy includingcached
    }
    else
    {
        found->flags=flags;
        found->path.assign(path);
        found->ordinal=ordinal;
    }
}

namespace ResourceIndexServices
{
    void AssignPath(std::string& destination, const char* path) { destination.assign(path); }
}
} }
