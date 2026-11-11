[CmdletBinding()]
param([ValidateSet('Personal','Upstream')] [string]$Source = 'Personal')
$ErrorActionPreference = 'Stop'
$workspacePath = Split-Path $PSScriptRoot -Parent
. (Join-Path $PSScriptRoot 'Workspace-Config.ps1')
$config = Get-WorkspaceConfig
$databasePath = if ($Source -eq 'Personal') {
    if (-not $config.personalDatabase) { throw 'Set the optional personalDatabase path in config/local.json, or select -Source Upstream after fetching references.' }
    $config.personalDatabase
} else {
    Join-Path $workspacePath 'repos/openje-re/ida/F3.exe.ida8.4.i64'
}
if (-not (Test-Path -LiteralPath $databasePath -PathType Leaf)) {
    throw "Analysis database is missing: $databasePath"
}
# Share one existing session and close owned references without saving.
(Resolve-Path -LiteralPath $databasePath).Path
