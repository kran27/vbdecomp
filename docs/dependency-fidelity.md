# Dependencies

| Library/tool | Version/source | Match status |
| --- | --- | --- |
| VC7 and CRT/STL | Visual Studio .NET; compiler 13.00.9466, linker 7.00.9466 | Original toolchain; selected CRT/STL code and layouts verified. |
| Platform SDK | November 2001, 5.1.3590.2 | All 177 named Windows imports and 17 Winsock ordinals match. |
| DirectX / D3DX | DirectX 8.1 SDK | 141 complete procedures match (42,953 code bytes). |
| CD3DFont | DirectX 8.1 SDK sample | Layout verified; 66-byte constructor matches. |
| Interplay ACM decoder | `Adecode.lib` from the [Descent 3 source release](https://github.com/DescentDevelopers/Descent3/blob/df209742fcda59d466b849eff9ee0e7b908d7d40/lib/win/Adecode.lib); version unknown | 28 complete routines and initialized data match exactly. |
| Small | 2.3.1 with F3 adaptations; [kran27/small-script](https://github.com/kran27/small-script) | Complete 8,480-byte AMX runtime matches. |
| Resource gzip / zlib | Unmodified upstream 1.1.4 | Version, stream layout and selected complete routines verified. |
| Game DLLs | [Original demo](https://vb.kran.gg/F3_Demo.rar) | Original files verified by hash and imports. |
| Formatting scanner | Microsoft CString-derived source from WTL 7.5; original version unknown | Provisional adaptation; no exact match. |

Download revisions and hashes: [dependency origins](../config/dependency-origins.json).
Setup: [build inputs](dependencies.md).
