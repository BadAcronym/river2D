param
(
    $build = "debug",
    [switch]$dontrun = $false
)

Write-Host "Building $build...`n"

$Platforms = "Win64", "Linux"
$Configurations = "debug", "release"
$libnames = "river2Dsoftware"
$targetname = "river2Dmapedit"
$targetlist = $libnames, $targetname
$target = ""

if(-Not(Test-Path "./obj/"))
{
    &mkdir "./obj/"
}

if(-Not(Test-Path "./bin/"))
{
    &mkdir "./bin/"
}

foreach($platform in $Platforms)
{
    foreach($config in $Configurations)
    {
        $objPath = "./obj/$platform" + "_$config"
        if(-Not(Test-Path $objPath))
        {
            &mkdir $objPath
        }

        $binPath = "./bin/$platform" + "_$config"
        if(-Not(Test-Path $binPath))
        {
            &mkdir $binPath
        }
    }
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

    $target = "./bin/Linux" + "_$build/$targetname"

    if(Test-Path $target)
    {
        &chmod +x $target
    }
}
elseIf($IsWindows)
{
    &premake5 vs2022

    &MSBuild ./build/$targetname.sln -p:Configuration=$build

    $target = "./bin/Win64" + "_$build/$targetname.exe"
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
