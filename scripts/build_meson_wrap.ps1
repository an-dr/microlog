#!/usr/bin/env pwsh
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

try {
    $repoRoot      = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
    $versionFile   = Join-Path $repoRoot 'version'
    $template      = Join-Path $repoRoot 'meson/meson.wrap.in'
    $replaceScript = Join-Path $repoRoot 'scripts/replace_variables.ps1'
    $wrapDir       = Join-Path $repoRoot 'install/wrap'
    $wrapFile      = Join-Path $wrapDir 'microlog.wrap'

    foreach ($f in @($versionFile,$template,$replaceScript)) {
        if (-not (Test-Path $f)) { throw "Missing required file: $f" }
    }

    $version = (Get-Content $versionFile -TotalCount 1).Trim()
    if (-not $version) { throw "Version file empty: $versionFile" }

    New-Item -ItemType Directory -Path $wrapDir -Force | Out-Null

    & $replaceScript -InputFile $template -OutputFile $wrapFile -Replacements @{ ULOG_VERSION = $version }

    Write-Host "Generated: $wrapFile"
    Get-Content $wrapFile | Write-Host
}
catch {
    Write-Host "Error: $($_.Exception.Message)"
    exit 1
}
