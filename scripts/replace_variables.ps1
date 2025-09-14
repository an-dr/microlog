# *************************************************************************
#
# Copyright (c) 2025 Andrei Gramakov. All rights reserved.
#
# This file is licensed under the terms of the MIT license.  
# For a copy, see: https://opensource.org/licenses/MIT
#
# site:    https://agramakov.me
# e-mail:  mail@agramakov.me
#
# *************************************************************************

# Script to replace variables in a file with values from a hashtable.
# It expects the input file to contain variables in the format @{variableName}@.
# Usage: .\replace_variables.ps1 -InputFile "path\to\input.txt" 
#                                -OutputFile "path\to\output.txt" 
#                                -Replacements @{ "variable1" = "value1"; "variable2" = "value2" }


param (
    [string]$InputFile,
    [string]$OutputFile,
    [hashtable]$Replacements
)

$content = Get-Content $InputFile -Raw

foreach ($key in $Replacements.Keys) {
    $pattern = "@$key@"
    $value = $Replacements[$key]
    $content = $content -replace [regex]::Escape($pattern), $value
}

# Ensure output directory exists
$outputDir = Split-Path -Parent $OutputFile
if (!(Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir | Out-Null
}

Set-Content -Path $OutputFile -Value $content
