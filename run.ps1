param
(
    $build = "debug",
    [switch]$dontrun = $false
)

Write-Host "Building $build...`n"

$solution = "river2D"
$targetname = "river2Dmapedit"
$target = ""

if(-Not(Test-Path "./obj/"))
{
    &mkdir "./obj/"
}

if(-Not(Test-Path "./bin/"))
{
    &mkdir "./bin/"
}

if(Test-Path "./vendor/imgsurf/run.ps1")
{
    Push-Location "./vendor/imgsurf/"
    &./run.ps1 $build -dontrun
    Pop-Location
}

if(-Not(Test-Path "./build/"))
{
    &mkdir "./build/"
}

if(-Not(Test-Path "./log/"))
{
    &mkdir "./log/"
}

&premake5 ecc

if($IsLinux)
{
    &premake5 gmake

    $makecfg = $build + "_linux"

    Push-Location "./build/"
    &make config=$makecfg
    Pop-Location

    $target = "./bin/$targetname" + "_linux/$build/$targetname"

    if(Test-Path $target)
    {
        &chmod +x $target
    }
}
elseIf($IsWindows)
{
    &premake5 vs2022

    &MSBuild ./build/$solution.sln -p:platform=windows -p:Configuration=$build

    $target = "./bin/$targetname" + "_win64/$build/$targetname.exe"

    #TODO: go through compile_commands and replace the defines like BUILD_LINUX with the appropriate ones
}

if($isWindows -and 0 -eq $LASTEXITCODE -and $build -eq "debug")
{
    Write-Host "`ngenerating rdi debug info..."

    Invoke-Expression "radbin --rdi $target"
}

if(0 -eq $LASTEXITCODE -and -not $dontrun)
{
    Write-Host "`nrunning $target..."
    Invoke-Expression $target
}
