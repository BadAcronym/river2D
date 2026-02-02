Invoke-Expression ".\run -dontrun"
if(-Not(Test-Path ".\bin\Win64_Debug\river2D.exe"))
{
    Write-Host "no executable to debug."
    return;
}

&raddbg
