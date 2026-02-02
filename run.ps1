param
(
    [Parameter(position=0,mandatory=$false)]
    $build = "DEBUG",
    [Parameter(position=1,mandatory=$false)]
    [switch]$dontrun = $false
)

Write-Host "Building $build...`n"

$Configurations = "Debug", "Release"
$Name = "river2D"

foreach($config in $Configurations)
{
    $objPath = "./obj/Win64" + "_$config"
    $binPath = "./bin/Win64" + "_$config"

    if(-Not(Test-Path $objPath))
    {
        &mkdir $objPath
    }

    if(-Not(Test-Path $binPath))
    {
        &mkdir $binPath
    }
}

if(-Not(Test-Path "./build/"))
{
    &mkdir "./build/"
}

&premake5 ecc
&premake5 vs2022

&MSBuild ./build/$Name.sln -p:Configuration=$build

$target = "./bin/Win64" + "_$build/$Name.exe"

if($LASTEXITCODE -eq 0 -and -not $dontrun)
{
    Write-Host "`nrunning $target..."

    Invoke-Expression $target
}
exit $LASTEXITCODE
