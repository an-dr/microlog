# Runs the CPM integration test on a commit that exists only locally.
#
# A pull request from a fork is in this position: its commit is not in
# an-dr/microlog, so the CPM test must not depend on fetching it from there.

$ErrorActionPreference = "Stop"

$repoDir = (Resolve-Path "$PSScriptRoot/../../..").Path
$workDir = Join-Path ([IO.Path]::GetTempPath()) "microlog-cpm-$([guid]::NewGuid())"
$exitCode = 1

try {
    git clone --quiet $repoDir $workDir
    if ($LASTEXITCODE -ne 0) { throw "git clone failed" }

    # Commit the working tree's CPM project so that uncommitted edits are tested.
    Copy-Item "$PSScriptRoot/CMakeLists.txt" "$workDir/tests/integration/cpm/CMakeLists.txt" -Force
    git -C $workDir add tests/integration/cpm/CMakeLists.txt
    git -C $workDir -c user.name=test -c user.email=test@example.com -c commit.gpgsign=false `
        commit --quiet --allow-empty -m "Local-only commit"
    if ($LASTEXITCODE -ne 0) { throw "git commit failed" }

    pwsh -NoProfile -File "$workDir/tests/integration/cpm/test_cpm.ps1"
    $exitCode = $LASTEXITCODE
} finally {
    Remove-Item -Recurse -Force $workDir -ErrorAction SilentlyContinue
}

if ($exitCode -ne 0) {
    Write-Host "[FAILED] CPM could not build a commit that exists only locally."
    exit $exitCode
}
Write-Host "[OK] CPM built a commit that exists only locally."
