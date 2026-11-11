[CmdletBinding()]
param(
    [string]$Target,
    [ValidateSet('Debug', 'Release')][string]$Configuration = 'Release',
    [string]$OutputDirectory,
    [switch]$CompileOnly,
    [switch]$PlanOnly,
    [switch]$Rebuild,
    [switch]$KeepGoing
)

# Compile canonical C++98/C sources with the pinned VC7 CL/LIB/LINK tools.
$ErrorActionPreference = 'Stop'
$workspace = Split-Path $PSScriptRoot -Parent
$settingsPath = Join-Path $workspace 'config/legacy-build.json'
$settings = Get-Content -LiteralPath $settingsPath -Raw | ConvertFrom-Json
if ($settings.schemaVersion -ne 1) { throw 'Unsupported legacy-build configuration' }
if (-not $Target) { $Target = $settings.defaultTarget }
if (-not $OutputDirectory) { $OutputDirectory = $settings.outputDirectory }

function FullPath([string]$Path, [string]$Base = $workspace) {
    if ([IO.Path]::IsPathRooted($Path)) { return [IO.Path]::GetFullPath($Path) }
    return [IO.Path]::GetFullPath((Join-Path $Base $Path))
}
function Hash([string]$Path) { return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant() }
function SaveJson([string]$Path, $Value) {
    [IO.File]::WriteAllText($Path, ($Value | ConvertTo-Json -Depth 40) + "`n", [Text.UTF8Encoding]::new($false))
}
function QuoteArgument([string]$Value) {
    if ($Value -notmatch '[\s"]') { return $Value }
    $quoted = [regex]::Replace($Value, '(\\*)"', { param($m) $m.Groups[1].Value + $m.Groups[1].Value + '\"' })
    $quoted = [regex]::Replace($quoted, '(\\+)$', '$1$1')
    return '"' + $quoted + '"'
}
function FragmentArguments([string]$Text) {
    foreach ($match in [regex]::Matches($Text, '(?:"[^"]*"|[^\s"])+')) {
        $match.Value.Trim('"')
    }
}

$graph = FullPath $settings.generatedDirectory
$output = FullPath (Join-Path $OutputDirectory $Configuration.ToLowerInvariant())
New-Item -ItemType Directory -Path $output -Force | Out-Null
$targetLog = Join-Path $output ($Target -replace '[^A-Za-z0-9_.-]', '_')
New-Item -ItemType Directory -Path $targetLog -Force | Out-Null
$manifestPath = FullPath $settings.targetManifest
$manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
$settingsDigest = Hash $settingsPath
$manifestDigest = Hash $manifestPath
if ($manifest.schemaVersion -ne 1) { throw 'Unsupported build-target manifest' }
$byID, $byName = @{}, @{}
foreach ($node in $manifest.targets) {
    $byID[$node.id] = $node
    $byName[$node.name] = $node
}
if (-not $PlanOnly) {
    $smallSourceRoot = FullPath 'src/dependencies/small-script'
    foreach ($kind in @('runtime', 'console')) {
        $destination = if ($kind -eq 'runtime') { 'small-amx-host' } else { 'small-console' }
        & (FullPath $settings.python) (Join-Path $PSScriptRoot "prepare_small_$kind.py") `
            --source-root $smallSourceRoot --output (Join-Path $graph $destination)
        if ($LASTEXITCODE -ne 0) { throw "Pinned Small $kind preparation failed" }
    }
}
$focus = $settings.sourceTargets.PSObject.Properties[$Target]
if ($focus) {
    $focus = $focus.Value
    $sourceTarget = $focus.target
    $CompileOnly = $CompileOnly -or $focus.compileOnly
} else { $sourceTarget = $Target }
$requestedTargets = @($sourceTarget)
foreach ($name in $requestedTargets) {
    if (-not $byName.ContainsKey($name)) { throw "No VC7 target named $name" }
}
$selected = @{}
function VisitTarget([string]$ID) {
    if ($selected.ContainsKey($ID)) { return }
    $selected[$ID] = $byID[$ID]
    if (-not $focus) { foreach ($dependency in $byID[$ID].dependencies) { VisitTarget $dependency.id } }
}
foreach ($name in $requestedTargets) { VisitTarget $byName[$name].id }
$artifactMap, $targetArtifacts = @{}, @{}
foreach ($node in $selected.Values) {
    if ($node.type -in @('STATIC_LIBRARY', 'EXECUTABLE')) {
        $artifact = @($node.artifacts | Where-Object { $_.path -match '\.(lib|exe)$' })[0]
        if (-not $artifact) { throw "No primary artifact for $($node.name)" }
        $built = Join-Path $output ([IO.Path]::GetFileName($artifact.path))
        $artifactMap[(FullPath $artifact.path $graph)] = $built
        $targetArtifacts[$node.id] = $built
    }
}
$units = @()
foreach ($node in @($selected.Values | Sort-Object name)) {
    $ordinal = 0
    foreach ($source in $node.sources) {
        if ($null -eq $source.compileGroupIndex) { continue }
        $sourcePath = FullPath $source.path
        if ($focus -and $source.path.Replace('\', '/') -notin $focus.sources) { continue }
        $group = $node.compileGroups[$source.compileGroupIndex]
        if ($group.language -notin @('C', 'CXX')) { throw "Unsupported primary source language: $($group.language)" }
        $directory = Join-Path $output ('objects/' + $node.name)
        New-Item -ItemType Directory -Path $directory -Force | Out-Null
        $basename = '{0:D4}-{1}' -f $ordinal, [IO.Path]::GetFileName($source.path)
        $units += [pscustomobject]@{ target = $node.name; targetID = $node.id; source = $sourcePath
            language = $group.language; group = $group; object = Join-Path $directory ($basename + '.obj')
            prefix = Join-Path $directory $basename }
        $ordinal++
    }
}
if ($focus -and $units.Count -ne @($focus.sources).Count) { throw 'Focused canonical source not uniquely present in the selected target' }
$plan = [ordered]@{ target = $Target; sourceTarget = $sourceTarget; configuration = $Configuration
    graph = $graph; output = $output; sourceTree = $workspace; primaryToolchain = ('VC7 13.00.9466 / LIB+LINK7.00.9466 / PSDK ' + $settings.toolchain.platformSdkVersion + ' / DX8.1')
    compileOnly = [bool]$CompileOnly; sources = @($units | Select-Object target, source, language, object)
    targets = @($selected.Values | Sort-Object name | Select-Object name, type)
    targetManifest = @{ path = $settings.targetManifest; sha256 = Hash $manifestPath } }
SaveJson (Join-Path $targetLog 'plan.json') $plan
Write-Host "VC7 plan: $Target, $($units.Count) canonical translation units; $output"
if ($PlanOnly) { return }

$tools = @{}
foreach ($name in @('compiler', 'librarian', 'linker')) {
    $path = FullPath $settings.toolchain.$name
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Missing supplied legacy $name`: $path" }
    $tools[$name] = $path
}
$environmentNames = @('PATH', 'INCLUDE', 'LIB', 'CL', '_CL_', 'LINK', '_LINK_')
$saved = @{}
foreach ($name in $environmentNames) { $saved[$name] = [Environment]::GetEnvironmentVariable($name, 'Process') }
$operations = [Collections.Generic.List[object]]::new()
$failures = 0
$completed = $false
$failureMessage = $null
$toolPins = @{}
foreach ($name in $tools.Keys) {
    $digest = Hash $tools[$name]
    if ($digest -ne $settings.toolchain.expectedSha256.$name) { throw "Supplied legacy $name identity changed: $($tools[$name])" }
    $toolPins[$name] = @{ path = $tools[$name]; sha256 = $digest }
}
$identity = Get-Content -LiteralPath (FullPath $settings.toolchain.identityManifest) -Raw | ConvertFrom-Json
foreach ($property in $identity.tools.PSObject.Properties) {
    if ((Hash (FullPath $property.Value.path)) -ne $property.Value.sha256) { throw "VC7 toolchain identity changed: $($property.Value.path)" }
}
& (FullPath $settings.python) (Join-Path $PSScriptRoot 'dependency_bundle.py') verify
if ($LASTEXITCODE -ne 0) { throw 'Dependency bundle verification failed; run scripts/Setup.ps1' }

function InvokeLegacyTool([string]$Kind, [string[]]$Arguments, [string]$Prefix) {
    $response = $Prefix + '.rsp'
    [IO.File]::WriteAllLines($response, @($Arguments | ForEach-Object { QuoteArgument $_ }), [Text.Encoding]::Default)
    & $tools[$Kind] "@$response" *> ($Prefix + '.log')
    $code = $LASTEXITCODE
    $operations.Add(@{ tool = $Kind; executable = $tools[$Kind]; arguments = $Arguments; response = $response; log = $Prefix + '.log'; exitCode = $code })
    if ($code -ne 0) {
        Get-Content -LiteralPath ($Prefix + '.log') | Where-Object { $_ -match '(error|fatal error|warning) [CDL][0-9]|LNK[0-9]' } | Select-Object -First 14 | Write-Host
    }
    return $code
}
function RemapArtifacts([string]$Value) { return $Value }
function LegacyPath([string]$Relative) { return Join-Path $output $Relative }
function FilePins([string[]]$Paths) {
    foreach ($path in $Paths) {
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Missing build input: $path" }
        @{ path = $path; sha256 = Hash $path }
    }
}
function GetGameExecutable {
    . (Join-Path $PSScriptRoot 'Workspace-Config.ps1')
    $configuration = Get-WorkspaceConfig
    return Join-Path $configuration.gameRoot 'F3.exe'
}
function LinkNativeTarget($Node) {
    $linkArguments = @('/NOLOGO', '/MACHINE:X86', '/INCREMENTAL:NO', ('/OUT:' + $targetArtifacts[$Node.id])) + @($settings.configurations.$Configuration.linkOptions)
    $linkArguments += @($units | Where-Object targetID -eq $Node.id | Select-Object -ExpandProperty object)
    foreach ($fragment in $Node.link.commandFragments) {
        if ($fragment.role -eq 'flags') {
            foreach ($flag in FragmentArguments $fragment.fragment) {
                if ($flag -match '^/SUBSYSTEM:|^/NODEFAULTLIB:|^/ENTRY:') { $linkArguments += $flag }
            }
        } elseif ($fragment.role -eq 'libraries') {
            foreach ($library in FragmentArguments $fragment.fragment) {
                $originalPath = FullPath $library $graph
                if ($artifactMap.ContainsKey($originalPath)) { $linkArguments += $artifactMap[$originalPath] }
                elseif ($imports.ContainsKey($originalPath)) { $linkArguments += $imports[$originalPath] }
                elseif ($library -match '^deps[/\\]') { $linkArguments += FullPath $library }
                elseif ([IO.Path]::IsPathRooted($library) -or $library -match '[/\\]') { throw "Unmapped link input: $library" }
                else { $linkArguments += $library }
            }
        } else { throw "Unsupported link fragment: $($fragment.role)" }
    }
    if ((InvokeLegacyTool 'linker' $linkArguments (Join-Path $targetLog ($Node.name + '-link'))) -ne 0) { throw "VC7 LINK failed for $($Node.name)" }
    $operations[$operations.Count - 1].outputs = @(FilePins @($targetArtifacts[$Node.id]))
}

Push-Location $workspace
try {
    $env:PATH = (Split-Path $tools.compiler) + ';' + (Join-Path (FullPath $settings.toolchain.platformSdkRoot) 'Bin') + ';' + $saved.PATH
    $env:INCLUDE = (@($settings.toolchain.includeDirectories | ForEach-Object { FullPath $_ }) -join ';')
    $env:LIB = (@($settings.toolchain.libraryDirectories | ForEach-Object { FullPath $_ }) -join ';')
    foreach ($name in @('CL', '_CL_', 'LINK', '_LINK_')) { [Environment]::SetEnvironmentVariable($name, $null, 'Process') }
    $imports, $linkedTargets = @{}, @{}
    foreach ($unit in $units) {
        $arguments = @($settings.compileOptions) + @($settings.configurations.$Configuration.compileOptions)
        $arguments += $(if ($unit.language -eq 'C') { '/TC' } else { '/TP' })
        if ($settings.targetCompileOptions) {
            $targetOptions = $settings.targetCompileOptions.PSObject.Properties[$unit.target]
            if ($targetOptions) {
                $configurationOptions = $targetOptions.Value.PSObject.Properties[$Configuration]
                if ($configurationOptions) { $arguments += @($configurationOptions.Value) }
            }
        }
        # Original translation units can have distinct compiler switches. Keep
        # these explicit in the primary configuration.
        if ($settings.sourceCompileOptions) {
            foreach ($sourceOptions in $settings.sourceCompileOptions.PSObject.Properties) {
                if ((FullPath $sourceOptions.Name) -eq $unit.source) {
                    $configurationOptions = $sourceOptions.Value.PSObject.Properties[$Configuration]
                    if ($configurationOptions) { $arguments += @($configurationOptions.Value) }
                }
            }
        }
        if ($settings.targetCompileDefinitions) {
            $extraDefines = $settings.targetCompileDefinitions.PSObject.Properties[$unit.target]
            if ($extraDefines) {
                foreach ($define in $extraDefines.Value) { $arguments += '/D' + $define }
            }
        }
        foreach ($define in $unit.group.defines) { $arguments += '/D' + (RemapArtifacts $define.define) }
        # Forced include headers remain explicit compilation inputs.
        foreach ($fragment in $unit.group.compileCommandFragments) {
            foreach ($flag in FragmentArguments $fragment.fragment) {
                if ($flag -match '^/FI.+') { $arguments += RemapArtifacts $flag }
            }
        }
        # Use the period SDK declarations.
        $arguments += '/I' + (FullPath $settings.officialDirectXInclude)
        foreach ($include in $unit.group.includes) {
            $arguments += '/I' + (RemapArtifacts (FullPath $include.path))
        }
        $arguments += @('/c', ('/Fo' + $unit.object), $unit.source)
        $statePath = $unit.prefix + '.state.json'
        $key = (@($arguments) + @(Hash $settingsPath) + @(Hash $PSCommandPath) +
            @(Hash (FullPath $settings.toolchain.identityManifest)) + @($toolPins.compiler.sha256)) -join "`n"
        $canReuse = -not $Rebuild -and (Test-Path -LiteralPath $statePath) -and (Test-Path -LiteralPath $unit.object)
        if ($canReuse) {
            $state = Get-Content -LiteralPath $statePath -Raw | ConvertFrom-Json
            $canReuse = $state.key -eq $key -and $state.objectSha256 -eq (Hash $unit.object)
            foreach ($property in $state.inputs.PSObject.Properties) {
                if (-not (Test-Path -LiteralPath $property.Name) -or (Hash $property.Name) -ne $property.Value) { $canReuse = $false; break }
            }
        }
        if ($canReuse) { Write-Host "Current: $($unit.target)/$([IO.Path]::GetFileName($unit.source))"; continue }
        Write-Host "CL7: $($unit.target)/$([IO.Path]::GetFileName($unit.source))"
        $before = Hash $unit.source
        $result = InvokeLegacyTool 'compiler' $arguments $unit.prefix
        if ($result -ne 0) {
            $failures++
            if (-not $KeepGoing) { break }
            continue
        }
        if ($before -ne (Hash $unit.source)) { throw "Canonical source changed during compilation: $($unit.source)" }
        if (-not (Test-Path -LiteralPath $unit.object)) { throw "CL7 returned success without object: $($unit.object)" }
        $sourceInputs = @{}
        $sourceInputs[$unit.source] = $before
        foreach ($line in Get-Content -LiteralPath ($unit.prefix + '.log')) {
            if ($line -match '^Note: including file:\s*(.+)$') {
                $header = FullPath $Matches[1].Trim()
                $sourceInputs[$header] = Hash $header
            }
        }
        SaveJson $statePath @{ key = $key; inputs = $sourceInputs; objectSha256 = Hash $unit.object }
    }
    if ($failures) { throw "$failures canonical translation unit(s) failed under VC7; see $targetLog/build.json and objects/*/*.log" }
    if (-not $CompileOnly) {
        foreach ($node in @($selected.Values | Where-Object type -eq 'STATIC_LIBRARY' | Sort-Object name)) {
            $objects = @($units | Where-Object targetID -eq $node.id | Select-Object -ExpandProperty object)
            if (-not $objects.Count) { throw "No rebuilt object input for archive $($node.name)" }
            $archiveArguments = @('/NOLOGO', '/MACHINE:X86', ('/OUT:' + $targetArtifacts[$node.id])) + $objects
            if ((InvokeLegacyTool 'librarian' $archiveArguments (Join-Path $targetLog ($node.name + '-archive'))) -ne 0) { throw "Legacy LIB failed for $($node.name)" }
            $operations[$operations.Count - 1].outputs = @(FilePins @($targetArtifacts[$node.id]))
        }
        if ($selected.ContainsKey('openvb')) {
            $game = GetGameExecutable
            & (Join-Path $PSScriptRoot 'Generate-Granny21ImportLibrary.ps1') `
                -OutputDirectory (LegacyPath 'granny21') -Game $game -LibraryTool $tools.librarian
            if ($LASTEXITCODE -ne 0) { throw 'Original Granny import generation failed' }
            $imports[(FullPath 'granny21/granny21.lib' $graph)] = LegacyPath 'granny21/granny21.lib'
        }
        foreach ($node in @($selected.Values | Where-Object type -eq 'EXECUTABLE' | Sort-Object name)) {
            if ($linkedTargets.ContainsKey($node.id)) { continue }
            LinkNativeTarget $node
            $linkedTargets[$node.id] = $true
        }
    }
    if ((Hash $settingsPath) -ne $settingsDigest -or (Hash $manifestPath) -ne $manifestDigest) { throw 'Build configuration changed during compilation' }
    $completed = $true
    Write-Host "Legacy $Target completed; no game executable was launched."
} catch {
    $failureMessage = $_.Exception.Message
    throw
} finally {
    SaveJson (Join-Path $targetLog 'build.json') @{ target = $Target; configuration = $Configuration
        tools = $toolPins; operations = @($operations); sourceTree = $workspace
        targetManifest = $settings.targetManifest; codeSnapshots = $false
        compileFailures = $failures; compileOnly = [bool]$CompileOnly; runExecuted = $false
        buildToolsExecuted = @($operations | Where-Object { $_.buildToolExecution }).Count -gt 0
        driverSha256 = Hash $PSCommandPath; configurationSha256 = $settingsDigest; targetManifestSha256 = $manifestDigest
        completed = $completed; failure = $failureMessage }
    foreach ($name in $environmentNames) { [Environment]::SetEnvironmentVariable($name, $saved[$name], 'Process') }
    Pop-Location
}
