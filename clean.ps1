if(Test-Path "./vendor/imgsurf/clean.ps1")
{
    pushd "./vendor/imgsurf/"
    &./clean.ps1
    popd
}

Write-Host "cleaning up river2D builds..." -Fore Yellow

if(Test-Path "./bin")
{
    rm "./bin/" -Recurse -Force
}

foreach($file in (gci *.o))
{
    if(Test-Path $file)
    {
        Remove-Item $file
    }
}

foreach($file in (gci *.obj))
{
    if(Test-Path $file)
    {
        Remove-Item $file
    }
}

foreach($file in (gci *.exe))
{
    if(Test-Path $file)
    {
        Remove-Item $file
    }
}

foreach($file in (gci *.lib))
{
    if(Test-Path $file)
    {
        Remove-Item $file
    }
}

foreach($file in (gci *.pdb))
{
    if(Test-Path $file)
    {
        Remove-Item $file
    }
}

Write-Host "cleaned river2D!`n" -Fore Green
