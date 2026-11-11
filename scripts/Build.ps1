[CmdletBinding()]
param(
    [ValidateSet('Debug','Release')] [string]$Configuration = 'Release',
    [string]$Target,
    [string]$OutputDirectory,
    [switch]$CompileOnly,
    [switch]$PlanOnly,
    [switch]$Rebuild,
    [switch]$KeepGoing
)
$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'Build-Legacy.ps1') @PSBoundParameters
