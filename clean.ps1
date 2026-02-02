if(Test-Path "vendor/imgsurf")
{
    Push-Location "vendor/imgsurf"
    &./clean.ps1
    Pop-Location
}

$toDelete =
    "./build/",
    "./bin/",
    "./obj/",
    "./log/"

Write-Host "cleaning the build..."

foreach($folder in $toDelete)
{
    if(Test-Path $folder)
    {
        Remove-Item $folder -Recurse -Force
    }
}

Write-Host "all clean!" -ForegroundColor Green
