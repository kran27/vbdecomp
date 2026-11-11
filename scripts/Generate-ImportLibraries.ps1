[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
$workspace = Split-Path $PSScriptRoot -Parent
$librarian = Join-Path $workspace 'deps/msvc70/bin/LIB.EXE'
if (-not (Test-Path -LiteralPath $librarian -PathType Leaf)) {
    throw 'The supplied VC7 librarian is required: deps/msvc70/bin/LIB.EXE'
}
$python = Join-Path $workspace '.venv\Scripts\python.exe'
if (-not (Test-Path -LiteralPath $python)) { throw 'Run scripts/Setup.ps1 to create the Python environment.' }
$savedPath = $env:PATH
try {
    $env:PATH = (Split-Path $librarian -Parent) + ';' + $savedPath
    & $python (Join-Path $PSScriptRoot 'inspect_environment.py') --generate-defs
    if ($LASTEXITCODE -ne 0) { throw 'DLL export verification failed.' }
    foreach ($name in @('d3d8','granny2','binkw32')) {
        $def = Join-Path $workspace "deps\imports\$name.def"
        $library = Join-Path $workspace "deps\imports\$name.lib"
        & $librarian /nologo /machine:x86 "/def:$def" "/out:$library"
        if ($LASTEXITCODE -ne 0) { throw "Import library generation failed: $name" }
    }
    & $python (Join-Path $PSScriptRoot 'fix_d3d8_import.py')
    if ($LASTEXITCODE -ne 0) { throw 'D3D8 stdcall name mapping failed.' }
    & $python (Join-Path $PSScriptRoot 'fix_bink_import.py')
    if ($LASTEXITCODE -ne 0) { throw 'Bink decorated import name mapping failed.' }
} finally {
    $env:PATH = $savedPath
}
