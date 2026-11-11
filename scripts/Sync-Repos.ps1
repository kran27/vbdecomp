[CmdletBinding()]
param([switch]$SourceOnly)
$ErrorActionPreference = 'Stop'
$workspace = Split-Path $PSScriptRoot -Parent
$lock = Get-Content -LiteralPath (Join-Path $workspace 'repos.lock.json') -Raw | ConvertFrom-Json
foreach ($source in $lock.workspaceSources) {
    $path = Join-Path $workspace $source.path
    if (-not (Test-Path -LiteralPath (Join-Path $path $source.requiredFile) -PathType Leaf)) {
        throw "Checked-in source is missing: $($source.path). Restore it from the workspace Git repository."
    }
}
Write-Host 'Checked-in game and engine source is ready; no patches or nested repository setup required.'
