
<#
.SYNOPSIS
    Generates a vcpkg registry port for microlog with the correct SHA512 hash.

.DESCRIPTION
    Downloads the GitHub source tarball for a given tag, computes its SHA512,
    and writes a ready-to-use portfile.cmake + vcpkg.json + usage into dist/vcpkg-port/.

    Can be run locally or from CI.

.PARAMETER Tag
    The git tag to build the port for (e.g. "v7.0.0").
    Defaults to the tag matching the version in the version file.

.EXAMPLE
    .\scripts\build_vcpkg_port.ps1
    .\scripts\build_vcpkg_port.ps1 -Tag v7.0.0
#>

param(
    [string]$Tag
)

$ErrorActionPreference = "Stop"
Set-Location "$PSScriptRoot/.."

$RepoRoot = (Resolve-Path ".").Path
$OutDir = Join-Path $RepoRoot "dist/vcpkg-port"

# Read version from source if no tag provided
if (-not $Tag) {
    $Version = (Get-Content (Join-Path $RepoRoot "version") -Raw).Trim()
    $Tag = "v$Version"
}

$Version = $Tag.TrimStart("v")
$Url = "https://github.com/an-dr/microlog/archive/refs/tags/${Tag}.tar.gz"

Write-Host "Tag:     $Tag"
Write-Host "Version: $Version"
Write-Host "URL:     $Url"

# Download and compute SHA512
Write-Host "Downloading tarball..."
$TempFile = [System.IO.Path]::GetTempFileName()
try {
    Invoke-WebRequest -Uri $Url -OutFile $TempFile -UseBasicParsing
    $Hash = (Get-FileHash -Path $TempFile -Algorithm SHA512).Hash.ToLower()
    Write-Host "SHA512:  $Hash"
} finally {
    Remove-Item $TempFile -ErrorAction SilentlyContinue
}

# Generate output
if (Test-Path $OutDir) { Remove-Item $OutDir -Recurse -Force }
New-Item -ItemType Directory -Path $OutDir | Out-Null

# portfile.cmake
$Portfile = @"
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO an-dr/microlog
    REF "$Tag"
    SHA512 $Hash
)

vcpkg_cmake_configure(
    SOURCE_PATH "`${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH "lib/cmake/microlog")

file(REMOVE_RECURSE "`${CURRENT_PACKAGES_DIR}/debug")
file(REMOVE_RECURSE "`${CURRENT_PACKAGES_DIR}/lib")
file(REMOVE "`${CURRENT_PACKAGES_DIR}/LICENSE")

vcpkg_install_copyright(FILE_LIST "`${SOURCE_PATH}/LICENSE")
"@

Set-Content -Path (Join-Path $OutDir "portfile.cmake") -Value $Portfile -NoNewline

# vcpkg.json (update version from overlay template)
$OverlayJson = Get-Content (Join-Path $RepoRoot "ports/microlog/vcpkg.json") -Raw
$RegistryJson = $OverlayJson -replace '"version":\s*"[^"]*"', "`"version`": `"$Version`""
Set-Content -Path (Join-Path $OutDir "vcpkg.json") -Value $RegistryJson -NoNewline

# usage file (copy from overlay port verbatim)
Copy-Item (Join-Path $RepoRoot "ports/microlog/usage") (Join-Path $OutDir "usage")

Write-Host "Generated port in: $OutDir"
