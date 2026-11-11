[CmdletBinding()]
param(
    [ValidateSet('Debug','Release')] [string]$Configuration = 'Release',
    [string]$DependencyBundle,
    [string]$GameArchive,
    [string]$GameRoot,
    [switch]$SkipBuild
)
$ErrorActionPreference = 'Stop'
$workspace = Split-Path $PSScriptRoot -Parent
$localConfig = Join-Path $workspace 'config/local.json'
if ($GameRoot) {
    $config = if (Test-Path -LiteralPath $localConfig) {
        Get-Content -LiteralPath $localConfig -Raw | ConvertFrom-Json
    } else { [pscustomobject]@{} }
    $config | Add-Member -NotePropertyName gameRoot -NotePropertyValue ([IO.Path]::GetFullPath($GameRoot)) -Force
    [IO.File]::WriteAllText($localConfig, ($config | ConvertTo-Json) + "`n", [Text.UTF8Encoding]::new($false))
}
& (Join-Path $PSScriptRoot 'Sync-Repos.ps1') -SourceOnly
$python = Join-Path $workspace '.venv/Scripts/python.exe'
if (-not (Test-Path -LiteralPath $python)) {
    & python -m venv (Join-Path $workspace '.venv')
    if ($LASTEXITCODE -ne 0) { throw 'Python 3.11+ environment creation failed' }
}
& $python -m pip install -r (Join-Path $workspace 'requirements.txt') --disable-pip-version-check
if ($LASTEXITCODE -ne 0) { throw 'Python dependency installation failed' }
$arguments = @((Join-Path $PSScriptRoot 'setup_inputs.py'))
if ($DependencyBundle) { $arguments += @('--dependency-bundle', [IO.Path]::GetFullPath($DependencyBundle)) }
if ($GameArchive) { $arguments += @('--game-archive', [IO.Path]::GetFullPath($GameArchive)) }
& $python @arguments
if ($LASTEXITCODE -ne 0) { throw 'Pinned dependency/game setup failed' }
& (Join-Path $PSScriptRoot 'Generate-ImportLibraries.ps1')
if (-not $SkipBuild) { & (Join-Path $PSScriptRoot 'Build.ps1') -Configuration $Configuration }
