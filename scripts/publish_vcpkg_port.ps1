
<#
.SYNOPSIS
    Publishes the microlog vcpkg port to a local vcpkg registry clone.

.DESCRIPTION
    Builds the port artifacts (via build_vcpkg_port.ps1), copies them into the
    vcpkg registry at $VcpkgPath, stages the changes, and updates the version
    database with `vcpkg x-add-version`.

    Requires vcpkg to be bootstrapped (vcpkg.exe must exist in $VcpkgPath).

.PARAMETER Tag
    The git tag to publish (e.g. "v7.0.2").
    Defaults to the version in the version file.

.PARAMETER VcpkgPath
    Path to the local vcpkg registry clone.
    Defaults to "../vcpkg" relative to the microlog repo root.

.EXAMPLE
    .\scripts\publish_vcpkg_port.ps1
    .\scripts\publish_vcpkg_port.ps1 -Tag v7.0.2
    .\scripts\publish_vcpkg_port.ps1 -VcpkgPath C:\src\vcpkg
#>

param(
    [string]$Tag,
    [string]$VcpkgPath
)

$ErrorActionPreference = "Stop"
Set-Location "$PSScriptRoot/.."
$RepoRoot = (Resolve-Path ".").Path

# Resolve version / tag
if (-not $Tag) {
    $Version = (Get-Content (Join-Path $RepoRoot "version") -Raw).Trim()
    $Tag = "v$Version"
}
$Version = $Tag.TrimStart("v")

# Resolve vcpkg path
if (-not $VcpkgPath) {
    $VcpkgPath = Join-Path $RepoRoot "../vcpkg"
}
$VcpkgPath = (Resolve-Path $VcpkgPath).Path
$VcpkgExe  = Join-Path $VcpkgPath "vcpkg.exe"

Write-Host "Tag:     $Tag"
Write-Host "Version: $Version"
Write-Host "Vcpkg:   $VcpkgPath"

# ── Step 1: Build port artifacts ─────────────────────────────────────────────
Write-Host ""
Write-Host "==> Building port artifacts..."
& (Join-Path $PSScriptRoot "build_vcpkg_port.ps1") -Tag $Tag

$PortSrc = Join-Path $RepoRoot "dist/vcpkg-port"
$PortDst = Join-Path $VcpkgPath "ports/microlog"

# ── Step 2: Copy port files into registry ────────────────────────────────────
Write-Host ""
Write-Host "==> Copying port to: $PortDst"
if (Test-Path $PortDst) { Remove-Item $PortDst -Recurse -Force }
Copy-Item $PortSrc $PortDst -Recurse

# ── Step 3: Stage port directory ─────────────────────────────────────────────
Write-Host ""
Write-Host "==> Staging port files..."
git -C $VcpkgPath add "ports/microlog"

# ── Step 4: Update version database ──────────────────────────────────────────
Write-Host ""
Write-Host "==> Updating version database..."
if (-not (Test-Path $VcpkgExe)) {
    Write-Error (
        "vcpkg.exe not found at '$VcpkgExe'.`n" +
        "Bootstrap vcpkg first:  .\bootstrap-vcpkg.bat`n" +
        "Then re-run this script."
    )
    exit 1
}
& $VcpkgExe format-manifest (Join-Path $PortDst "vcpkg.json")
git -C $VcpkgPath add "ports/microlog"

& $VcpkgExe x-add-version microlog --overwrite-version

# ── Step 5: Stage version database changes ───────────────────────────────────
Write-Host ""
Write-Host "==> Staging version database..."
git -C $VcpkgPath add "versions/"

Write-Host ""
Write-Host "Done. Changes staged in: $VcpkgPath"
Write-Host "Review : git -C '$VcpkgPath' diff --cached"
Write-Host "Commit : git -C '$VcpkgPath' commit -m 'Add microlog $Version'"
