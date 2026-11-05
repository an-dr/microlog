
<#
.SYNOPSIS
    Publishes the microlog port update to the an-dr/vcpkg fork.

.DESCRIPTION
    Builds the release port artifacts, clones an-dr/vcpkg, creates a branch
    named microlog/v<version>, copies the port files, updates the version
    database, commits, and pushes. No PR is opened - submit it manually.

    Requires:
      - git available on PATH with push access to an-dr/vcpkg
        (set GH_TOKEN or configure credentials before running)
      - vcpkg bootstrapped in the cloned repo (done automatically)

.PARAMETER Tag
    The git tag of the microlog release (e.g. "v7.0.2").
    Defaults to the version in the version file.

.PARAMETER WorkDir
    Directory where the vcpkg fork will be cloned.
    Defaults to a temp directory.

.EXAMPLE
    .\scripts\submit_vcpkg_pr.ps1
    .\scripts\submit_vcpkg_pr.ps1 -Tag v7.0.2
#>

param(
    [string]$Tag,
    [string]$WorkDir
)

$ErrorActionPreference = "Stop"
Set-Location "$PSScriptRoot/.."
$RepoRoot = (Resolve-Path ".").Path

# ── Resolve tag / version ─────────────────────────────────────────────────────
if (-not $Tag) {
    $Version = (Get-Content (Join-Path $RepoRoot "version") -Raw).Trim()
    $Tag = "v$Version"
}
$Version = $Tag.TrimStart("v")

$ForkRepo  = "an-dr/vcpkg"
$Branch    = "microlog/v$Version"
$CommitMsg = "[microlog] Add version $Version"

Write-Host "Tag:     $Tag"
Write-Host "Version: $Version"
Write-Host "Fork:    $ForkRepo"
Write-Host "Branch:  $Branch"

# ── Step 1: Build port artifacts ─────────────────────────────────────────────
Write-Host ""
Write-Host "==> Building port artifacts..."
& (Join-Path $PSScriptRoot "build_vcpkg_port.ps1") -Tag $Tag
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$PortSrc = Join-Path $RepoRoot "dist/vcpkg-port"

# ── Step 2: Sync fork master with upstream ────────────────────────────────────
# Uses the GitHub merge-upstream API so the fork is up-to-date before we branch.
# Requires GH_TOKEN (or git credential) with push access to $ForkRepo.
Write-Host ""
Write-Host "==> Syncing $ForkRepo master with microsoft/vcpkg..."
$SyncHeaders = @{ "Accept" = "application/vnd.github+json" }
if ($env:GH_TOKEN) {
    $SyncHeaders["Authorization"] = "Bearer $env:GH_TOKEN"
}
$SyncBody = '{"branch":"master"}'
try {
    $SyncResp = Invoke-RestMethod `
        -Method Post `
        -Uri "https://api.github.com/repos/$ForkRepo/merge-upstream" `
        -Headers $SyncHeaders `
        -ContentType "application/json" `
        -Body $SyncBody
    Write-Host "Sync result: $($SyncResp.message)"
} catch {
    # A 409 means master is already up-to-date - not a failure.
    if ($_.Exception.Response.StatusCode.value__ -eq 409) {
        Write-Host "Fork master already up-to-date."
    } else {
        throw
    }
}

# ── Step 3: Clone the (now-synced) fork ──────────────────────────────────────
if (-not $WorkDir) {
    $WorkDir = Join-Path ([System.IO.Path]::GetTempPath()) "vcpkg-publish-$Version"
}
$VcpkgClone = Join-Path $WorkDir "vcpkg"

Write-Host ""
Write-Host "==> Cloning $ForkRepo into $VcpkgClone ..."
if (Test-Path $VcpkgClone) {
    Remove-Item $VcpkgClone -Recurse -Force
}
New-Item -ItemType Directory -Path $WorkDir -Force | Out-Null

# Shallow clone - we only need the current tree, not full history.
git clone --depth 1 "https://github.com/$ForkRepo.git" $VcpkgClone
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

# ── Step 4: Create branch ─────────────────────────────────────────────────────
Write-Host ""
Write-Host "==> Creating branch $Branch ..."
git -C $VcpkgClone checkout -b $Branch
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

# ── Step 5: Copy port files ───────────────────────────────────────────────────
$PortDst = Join-Path $VcpkgClone "ports/microlog"
Write-Host ""
Write-Host "==> Copying port to $PortDst ..."
if (Test-Path $PortDst) { Remove-Item $PortDst -Recurse -Force }
Copy-Item $PortSrc $PortDst -Recurse
git -C $VcpkgClone add "ports/microlog"

# ── Step 6: Bootstrap vcpkg and update version database ───────────────────────
Write-Host ""
Write-Host "==> Bootstrapping vcpkg..."
if ($IsWindows) {
    $BootstrapScript = Join-Path $VcpkgClone "bootstrap-vcpkg.bat"
    cmd /c $BootstrapScript -disableMetrics
} else {
    $BootstrapScript = Join-Path $VcpkgClone "bootstrap-vcpkg.sh"
    bash $BootstrapScript -disableMetrics
}
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$VcpkgExe = if ($IsWindows) {
    Join-Path $VcpkgClone "vcpkg.exe"
} else {
    Join-Path $VcpkgClone "vcpkg"
}

Write-Host ""
Write-Host "==> Formatting manifest..."
& $VcpkgExe format-manifest (Join-Path $PortDst "vcpkg.json")
git -C $VcpkgClone add "ports/microlog"

Write-Host ""
Write-Host "==> Updating version database..."
& $VcpkgExe x-add-version microlog --overwrite-version
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
git -C $VcpkgClone add "versions/"

# ── Step 7: Commit ────────────────────────────────────────────────────────────
Write-Host ""
Write-Host "==> Committing..."
git -C $VcpkgClone commit -m $CommitMsg
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

# ── Step 8: Push branch to fork ──────────────────────────────────────────────
Write-Host ""
Write-Host "==> Pushing branch to $ForkRepo ..."
git -C $VcpkgClone push origin $Branch --force-with-lease
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host ""
Write-Host "[OK] Branch '$Branch' pushed to $ForkRepo."
Write-Host "     Open a PR manually at: https://github.com/$ForkRepo/compare/$Branch"
