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
        Remove-Item $folder -Recurse
    }
}

Write-Host "all clean!" -ForegroundColor Green
