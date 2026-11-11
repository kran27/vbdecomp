[CmdletBinding()]
param(
    [string]$OutputDirectory,
    [string]$Game,
    [string]$LibraryTool
)
$ErrorActionPreference = 'Stop'
$workspace = Split-Path $PSScriptRoot -Parent
if (-not $Game) {
    . (Join-Path $PSScriptRoot 'Workspace-Config.ps1')
    $Game = Join-Path (Get-WorkspaceConfig).gameRoot 'F3.exe'
}
if (-not $OutputDirectory) { $OutputDirectory = Join-Path $workspace 'build\dependencies\granny21' }
$OutputDirectory = [System.IO.Path]::GetFullPath($OutputDirectory)
New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
$python = Join-Path $workspace '.venv\Scripts\python.exe'
if (-not $LibraryTool) { $LibraryTool = Join-Path $workspace 'deps\msvc70\bin\lib.exe' }
$definition = Join-Path $OutputDirectory 'granny21.def'
$library = Join-Path $OutputDirectory 'granny21.lib'

# LIB decorates the public x86 C symbol once. Its default IMPORT_NAME_NOPREFIX
# would then remove that underscore at DLL lookup, but Granny2.1 exports it.
# Keep the compiler's _GrannyFoo@N symbol and change only the documented short
# import name mode to IMPORT_NAME, preserving the DLL's exact _GrannyFoo@N name.
$prepare = @'
from pathlib import Path
import pefile,re,sys
game,out=map(Path,sys.argv[1:])
target=pefile.PE(str(game))
runtime=pefile.PE(str(game.parent/'granny2.dll'))
exports={s.name.decode('ascii') for s in runtime.DIRECTORY_ENTRY_EXPORT.symbols if s.name}
names=[i.name.decode('ascii') for d in target.DIRECTORY_ENTRY_IMPORT
       if d.dll.lower()==b'granny2.dll' for i in d.imports]
assert len(names)==43 and len(set(names))==43
assert all(re.fullmatch(r'_Granny\w+@\d+',n) and n in exports for n in names)
(out/'granny21.def').write_text('LIBRARY granny2.dll\nEXPORTS\n'+
                              ''.join('    '+n[1:]+'\n' for n in names))
'@
& $python -c $prepare $Game $OutputDirectory
if ($LASTEXITCODE -ne 0) { throw 'Granny2.1 DLL/import verification failed.' }
& $libraryTool /nologo /machine:x86 "/def:$definition" "/out:$library"
if ($LASTEXITCODE -ne 0) { throw 'Granny2.1 import library generation failed.' }

$finish = @'
from pathlib import Path
import hashlib,json,struct,sys
game,out,tool,script=map(Path,sys.argv[1:])
p=out/'granny21.lib'; data=bytearray(p.read_bytes())
assert data[:8]==b'!<arch>\n'
offset=8;names=[]
while offset<len(data):
    assert data[offset+58:offset+60]==b'`\n'
    size=int(data[offset+48:offset+58].decode('ascii').strip())
    start=offset+60;body=data[start:start+size]
    if body[:4]==b'\0\0\xff\xff':
        assert struct.unpack_from('<H',body,6)[0]==0x14c
        name,dll=body[20:].split(b'\0')[:2]
        assert name.startswith(b'_Granny') and dll==b'granny2.dll'
        flags=struct.unpack_from('<H',body,18)[0]
        assert flags>>2==2
        struct.pack_into('<H',data,start+18,(flags&~0x1c)|(1<<2))
        names.append(name.decode('ascii'))
    offset=start+size+(size%2)
assert len(names)==43
p.write_bytes(data)
def pin(q):
    return {'path':str(q.resolve()),'sha256':hashlib.sha256(q.read_bytes()).hexdigest()}
receipt={'runtime':'shipped Granny2.1.0.5','imports':names,
         'adaptation':'C stdcall symbol decoration once; exact decorated DLL export IMPORT_NAME',
         'inputs':[pin(q) for q in [game,game.parent/'granny2.dll',tool,script]],
         'outputs':[pin(out/'granny21.def'),pin(p)]}
(out/'granny21-imports.json').write_text(json.dumps(receipt,indent=2)+'\n')
print(str(p))
'@
& $python -c $finish $Game $OutputDirectory $libraryTool $PSCommandPath
if ($LASTEXITCODE -ne 0) { throw 'Granny2.1 exact DLL-name mapping failed.' }
