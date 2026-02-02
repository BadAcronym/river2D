$toDelete =
    "./build/",
    "./bin/",
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
