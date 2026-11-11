[CmdletBinding()]
param([string]$ArtifactDirectory = 'build/toolchain-check')
$ErrorActionPreference = 'Stop'
$workspace = Split-Path $PSScriptRoot -Parent
$settingsPath = Join-Path $workspace 'config/legacy-build.json'
$settings = Get-Content -LiteralPath $settingsPath -Raw | ConvertFrom-Json
$sdk = Join-Path $workspace $settings.toolchain.platformSdkRoot
$compiler = Join-Path $workspace 'deps/msvc70'
$output = [IO.Path]::GetFullPath((Join-Path $workspace $ArtifactDirectory))
$buildRoot = [IO.Path]::GetFullPath((Join-Path $workspace 'build')) + [IO.Path]::DirectorySeparatorChar
if (-not $output.StartsWith($buildRoot, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Toolchain checks must stay inside the workspace build directory'
}
New-Item -ItemType Directory -Path $output -Force | Out-Null
$python = Join-Path $workspace '.venv/Scripts/python.exe'
& $python (Join-Path $PSScriptRoot 'dependency_bundle.py') verify
if ($LASTEXITCODE -ne 0) { throw 'Original dependency verification failed' }
$resource = Join-Path $output 'probe.res'
& (Join-Path $sdk 'Bin/RC.Exe') /x /i (Join-Path $sdk 'include') /fo $resource (Join-Path $workspace 'src/vc7_sdk_probe.rc') *> (Join-Path $output 'resource.log')
if ($LASTEXITCODE -ne 0) { throw 'Original SDK resource compiler check failed' }
$results = @()
foreach ($mode in @('MT', 'MD', 'MTd', 'MDd')) {
    $executable = Join-Path $output ($mode + '.exe')
    $object = Join-Path $output ($mode + '.obj')
    $pdb = Join-Path $output ($mode + '.pdb')
    & (Join-Path $PSScriptRoot 'Invoke-Vc7.ps1') /nologo /EHsc /Zi /showIncludes "/$mode" "/Fd$pdb" (Join-Path $workspace 'src/vc7_sdk_probe.cpp') "/Fo$object" "/Fe$executable" /link /MACHINE:X86 /INCREMENTAL:NO $resource user32.lib gdi32.lib winmm.lib ole32.lib advapi32.lib uuid.lib d3d8.lib d3dx8.lib /NODEFAULTLIB:LIBC /NODEFAULTLIB:libci /NODEFAULTLIB:amdmath *> (Join-Path $output ($mode + '.log'))
    if ($LASTEXITCODE -ne 0) { throw "VC7 /$mode compile/link check failed; see $output" }
    $results += @{ mode = $mode; compiled = $true; linked = $true; executed = $false;
        sha256 = (Get-FileHash -LiteralPath $executable -Algorithm SHA256).Hash.ToLowerInvariant() }
    Write-Host "/$mode compile/link passed; no executable launched"
}
@{ passed = $true; compiler = '13.00.9466'; sdk = '5.1.3590.2'; probes = $results;
    runtimeValidated = $false; scope = 'Static compiler/linker/resource checks only' } |
    ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $output 'report.json') -Encoding utf8
