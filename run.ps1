param
(
    [Parameter(position=0,Mandatory=$false)]
    $build = "debug"
)

Write-Host "Building $build...`n"

$Platforms = "Win64", "Linux"
$Configurations = "debug", "release"
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
        $binPath = "./bin/$platform" + "_$config"

        if(-Not(Test-Path $objPath))
        {
            &mkdir $objPath
        }

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

    $target = "./bin/Linux" + "_$build/river2Dmapedit"

    if(Test-Path $target)
    {
        &chmod +x $target
    }
}
elseIf($IsWindows)
{
    &premake5 vs2022

    &MSBuild ./build/River.sln -p:Configuration=$build

    $target = "./bin/Win64" + "_$build/river2Dmapedit.exe"
}

if($isWindows -and 0 -eq $LASTEXITCODE -and $build -eq "debug")
{
    Write-Host "`ngenerating rdi debug info..."

    Invoke-Expression "radbin --rdi $target"
}

if(0 -eq $LASTEXITCODE)
{
    Write-Host "`nrunning $target..."
    Invoke-Expression $target
}
