if(Test-Path "vendor/imgsurf")
{
    Push-Location "vendor/imgsurf"
    &./clean.ps1
    Pop-Location
}

$toDelete =
    "./build/",
    "./bin/river2Dmapedit_linux/",
    "./obj/",
    "./log/",
    "./compile_commands.json"

Write-Host "cleaning the build..."

foreach($folder in $toDelete)
{
    if(Test-Path $folder)
    {
        Remove-Item $folder -Recurse
    }
}

Write-Host "all clean!" -ForegroundColor Green
