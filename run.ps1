param
(
    $build = "release",
    [switch]$compile_only = $false
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

function Get-CompiledImgloader()
{
    if(Test-Path ".\vendor\imgsurf\run.ps1")
    {
        pushd ".\vendor\imgsurf\"
        .\run.ps1 $build --compile-only
        if(0 -ne $LASTEXITCODE)
        {
            exit -1
        }
        popd
    }
    else
    {
        Write-Host "\033[31m\nERROR: can't find imgsurf run script.\033[0m"
    }
}

function Get-Compileprep()
{
    Write-Host ""
    Write-Host "\033[36mcompiling imgsurf...\033[0m"
    Write-Host ""
    premake5 ecc
    premake5 vs2022
}

if($build -eq "debug")
{
    Get-CompiledImgloader
    Get-Compileprep
    pushd ".\build\"
    &MSBuild river2D.sln -p:Configuration=$build -p:Platform=windows
    popd
}
elseif($build -eq "release")
{
    Get-CompiledImgloader
    Get-Compileprep
    pushd ".\build\"
    &MSBuild river2D.sln -p:Configuration=$build -p:Platform=windows
    popd
}
elseif($build -eq "asan")
{
    Get-CompiledImgloader
    Get-Compileprep
    pushd ".\build\"
    &MSBuild river2D.sln -p:Configuration=$build -p:Platform=windows
    popd
}
else
{
    Write-Host "\033[31m\nERROR: invalid make config: $build.\033[0m"
    exit -2;
}

if(0 -ne $LASTEXITCODE)
{
    Write-Host "\033[31m\nERROR: failed to compile.\n\033[0m"
    popd
    exit -1
}

Write-Host "\n"

if($compile_only)
{
    exit 0
}

popd
