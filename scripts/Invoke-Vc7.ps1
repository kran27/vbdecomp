[CmdletBinding(PositionalBinding = $false)]
param(
    [Parameter(Position = 0, ValueFromRemainingArguments)] [string[]]$CompilerArguments
)

# Supplied primary compiler/SDK. Build.ps1 uses the same tools for the game.
$ErrorActionPreference = 'Stop'
$workspace = Split-Path $PSScriptRoot -Parent
$settings = Get-Content -LiteralPath (Join-Path $workspace 'config\legacy-build.json') -Raw | ConvertFrom-Json
$compilerRoot = Join-Path $workspace 'deps\msvc70'
$compilerPath = Join-Path $compilerRoot 'bin/CL.EXE'
if ((Get-FileHash -LiteralPath $compilerPath -Algorithm SHA256).Hash.ToLowerInvariant() -ne $settings.toolchain.expectedSha256.compiler) { throw 'VC7 compiler identity differs' }
$SdkRoot = $settings.toolchain.platformSdkRoot
if (-not [IO.Path]::IsPathRooted($SdkRoot)) { $SdkRoot = Join-Path $workspace $SdkRoot }
$SdkRoot = [IO.Path]::GetFullPath($SdkRoot)
$directXRoot = Join-Path $workspace 'deps\dx81'
foreach ($required in @((Join-Path $compilerRoot 'bin\cl.exe'), (Join-Path $SdkRoot 'include\Windows.h'), (Join-Path $SdkRoot 'Lib\User32.Lib'))) {
    if (-not (Test-Path -LiteralPath $required -PathType Leaf)) { throw "Missing reference toolchain file: $required" }
}
$variables = @('PATH', 'INCLUDE', 'LIB', 'CL', '_CL_', 'LINK', '_LINK_')
$saved = @{}
foreach ($name in $variables) { $saved[$name] = [Environment]::GetEnvironmentVariable($name, 'Process') }
try {
    $env:PATH = "$compilerRoot\bin;$SdkRoot\Bin;" + $saved.PATH
    $env:INCLUDE = "$compilerRoot\include;$SdkRoot\include;$directXRoot\include"
    $env:LIB = "$SdkRoot\Lib;$compilerRoot\lib;$directXRoot\lib;$workspace\deps\imports"
    foreach ($name in @('CL', '_CL_', 'LINK', '_LINK_')) { [Environment]::SetEnvironmentVariable($name, $null, 'Process') }
    & (Join-Path $compilerRoot 'bin\cl.exe') @CompilerArguments
    $result = $LASTEXITCODE
} finally {
    foreach ($name in $variables) { [Environment]::SetEnvironmentVariable($name, $saved[$name], 'Process') }
}
exit $result
