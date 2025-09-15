#!/usr/bin/env pwsh

param(
    [string]$Version,
    [switch]$Clean
)
$ErrorActionPreference = 'Stop'

$scriptDir = Split-Path -Parent $PSCommandPath
$root = Split-Path -Parent $scriptDir

if (-not $Version) {
    $vf = Join-Path $root 'version'
    if (Test-Path $vf) {
        $Version = (Get-Content $vf -Raw).Trim()
    } else {
        Write-Error "Version not specified and version file missing"
        exit 1
    }
}
if ($Version -notmatch '^\d+\.\d+\.\d+') { Write-Error "Invalid version: $Version"; exit 1 }

$libName = 'microlog'
$outDir = Join-Path $root 'install/arduino'
$libDir = Join-Path $outDir $libName

if ($Clean) { Remove-Item $libDir -Recurse -Force -ErrorAction SilentlyContinue }

@($libDir, "$libDir/src", "$libDir/examples/BasicLogging") | ForEach-Object {
    New-Item -ItemType Directory -Path $_ -Force | Out-Null
}

function Copy-WithVersion($src, $dst) {
    (Get-Content $src -Raw) -replace '@ULOG_VERSION@', $Version | Set-Content $dst
}

Copy-WithVersion (Join-Path $root 'arduino/library.properties') (Join-Path $libDir 'library.properties')
Copy-WithVersion (Join-Path $root 'include/ulog.h')              (Join-Path $libDir 'src/ulog.h')
Copy-WithVersion (Join-Path $root 'src/ulog.c')                  (Join-Path $libDir 'src/ulog.c')

Copy-Item (Join-Path $root 'arduino/keywords.txt') $libDir
Copy-Item (Join-Path $root 'README.md')            $libDir
Copy-Item (Join-Path $root 'LICENSE')              $libDir
Copy-Item (Join-Path $root 'arduino/BasicLogging.ino.template') (Join-Path $libDir 'examples/BasicLogging/BasicLogging.ino')

$zip = Join-Path $outDir "$libName-$Version-arduino.zip"
Remove-Item $zip -Force -ErrorAction SilentlyContinue
Compress-Archive -Path $libDir -DestinationPath $zip

Write-Host "Done: $libDir"
Write-Host "ZIP:  $zip"
