# Paths supplied by the user are optional; relative paths resolve from the checkout.
function Get-WorkspaceConfig {
    $root = Split-Path $PSScriptRoot -Parent
    $values = @{ gameRoot = 'deps/game'; personalDatabase = '' }
    $local = Join-Path $root 'config/local.json'
    if (Test-Path -LiteralPath $local) {
        $settings = Get-Content -LiteralPath $local -Raw | ConvertFrom-Json
        foreach ($key in @('gameRoot', 'personalDatabase')) {
            if ($settings.$key) { $values[$key] = [string]$settings.$key }
        }
    }
    foreach ($key in @('gameRoot', 'personalDatabase')) {
        if ($values[$key] -and -not [IO.Path]::IsPathRooted($values[$key])) {
            $values[$key] = [IO.Path]::GetFullPath((Join-Path $root $values[$key]))
        }
    }
    return [pscustomobject]$values
}
