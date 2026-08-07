param
(
    [Parameter(Position = 0)][string]$build,
    [Parameter(Position = 1)][string]$compile_only
)

if(-Not(Test-Path ".\obj\"))
{
    &mkdir .\obj\
}

if(-Not(Test-Path ".\build\"))
{
    &mkdir .\build\
}

if(-Not(Test-Path ".\bin\"))
{
    &mkdir .\bin\
}

if($build -eq $null -or $build -eq "")
{
    $build = "release"
}

if($build -eq "asan" -or $build -eq "debug" -or $build -eq "release")
{

    if(Test-Path "./vendor/imgsurf/run.ps1")
    {
        pushd "./vendor/imgsurf/"
        &./run.ps1 $build --compile-only
        if($LASTEXITCODE -ne 0)
        {

            Write-Host "`nERROR: failed to compile imgsurf.`n" -ForegroundColor Red
            popd
            exit -3;
        }
        popd
    }
    else
    {
        Write-Host "ERROR: can't find imgsurf's run script." -ForegroundColor Red
    }

    Write-Host "`ncompiling river2D...`n" -Fore Cyan

    premake5 gmake
    pushd "./build/"
    make config=$build`_windows
    popd
}
else
{
    Write-Host "ERROR: invalid make config: '$build'." -ForegroundColor Red
    exit -2;
}

if($LASTEXITCODE -ne 0)
{
    Write-Host "`nERROR: failed to compile river2D.`n" -ForegroundColor Red
    exit -1;
}

Write-Host "`n"

if($compile_only -eq "--compile-only")
{
    exit 0;
}
