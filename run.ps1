param
(
    [Parameter(Position = 0)][string]$build,
    [Parameter(Position = 1)][string]$compile_only
)

if(-Not(Test-Path "./bin/" -PathType Container))
{
    mkdir "./bin/"
}

if($build -eq $null -or $build -eq "")
{
    $build = "release"
}

$args_always=@("-DBUILD_WINDOWS",
"vendor/imgsurf/vendor/datasurf/src/datasurf_formats.c",
"vendor/imgsurf/vendor/datasurf/src/datasurf_algo_deflate.c",
"vendor/imgsurf/vendor/datasurf/vendor/puddle/src/win32_pd_path.c",
"vendor/imgsurf/vendor/datasurf/vendor/puddle/src/string_view.c",
"-Iinclude",
"-Ivendor/imgsurf/include",
"-Ivendor/imgsurf/vendor/datasurf/include",
"-Ivendor/imgsurf/vendor/datasurf/vendor/puddle/include",
"-std=c99",
"-Wextra", "-Wall", "-Wpedantic", "-Wconversion", "-Wshadow", "-Wsign-compare",
"-Wtype-limits", "-Wunused",
"-Wno-unsafe-buffer-usage", "-Wno-declaration-after-statement", "-Wno-vla",
"-Wno-implicit-void-ptr-cast")

$args_common=@("src/river2Dcommon_main.c",
"src/win32_river2Dcommon.c",
"src/river2D_util.c",
"-c")

$args_software=@("src/win32_river2Dsoftware.c",
"./vendor/imgsurf/src/imgsurf_main.c",
"./vendor/imgsurf/src/imgsurf_format_bmp.c",
"./vendor/imgsurf/src/imgsurf_format_qoi.c",
"./vendor/imgsurf/src/imgsurf_format_png.c",
"-Lbin/$build/",
"-lriver2Dcommon",
"-shared",
"-lgdi32",
"-luser32")

$args_release=@("-O2")

$args_debug=@("-DDEBUG", "-gcodeview", "-O0")
$args_debug_cl=@("/DDEBUG", "/Zi", "/Od")

$args_asan=$args_debug_cl+@("/clang:-std=c99", "/DASAN",
"/fsanitize=address", "/MD",
"/link", "/SUBSYSTEM:CONSOLE")

function compile
{
    param( [string[]]$1 )

    Write-Host "identifying a compiler..."

    if($build -eq "asan")
    {
        if(-Not(Get-Command clang -ErrorAction SilentlyContinue))
        {
            Write-Host "ERROR: clang-cl needed for address sanitization." -Fore Red
        }
        $script:compiler="clang-cl"
    }
    elseif(Get-Command clang -ErrorAction SilentlyContinue)
    {
        Write-Host "found clang."
        $script:compiler="clang"
    }
    elseif(Get-Command gcc -ErrorAction SilentlyContinue)
    {
        Write-Host "found gcc."
        $script:compiler="gcc"
    }
    else
    {
        Write-Host "ERROR: no suitable compiler found." -Fore Red
    }

    if(-Not (Test-Path "./bin/$build/" -PathType Container))
    {
        mkdir "./bin/$build/"
    }

    Write-Host ""
    Write-Host "compiling river2Dcommon..." -Fore Cyan
    Write-Host ""

    Write-Host "compiling $build build with the following command:"
    $final=($1 + $args_common)
    Write-Host "$script:compiler $final"
    &$script:compiler @final
    if($LASTEXITCODE -ne 0)
    {
        Write-Host "`nERROR: $script:compiler failed to compile river2Dcommon.`n" -Fore Red
        exit -1
    }
    Write-Host ""
    Write-Host "creating static library river2Dcommon.lib..." -Fore Cyan
    Write-Host ""
    &ar rcs ./bin/$build/river2Dcommon.lib river2D_util.o string_view.o river2Dcommon_main.o win32_river2Dcommon.o win32_pd_path.o
    if($LASTEXITCODE -ne 0)
    {
        Write-Host "`nERROR: ar failed to create static library.`n" -Fore Red
        exit -1
    }

    Write-Host ""
    Write-Host "compiling river2Dsoftware..." -Fore Cyan
    Write-Host ""
    Write-Host "compiling $build build with the following command:"
    $final=($1 + $args_software)
    Write-Host "$script:compiler $final"
    &$script:compiler @final
    if($LASTEXITCODE -ne 0)
    {
        Write-Host "`nERROR: $script:compiler failed to compile river2Dsoftware.`n" -Fore Red
        exit -1
    }
    Move-Item ./a.exe ./bin/$build/river2Dsoftware.dll -Force
    Move-Item ./a.lib ./bin/$build/river2Dsoftware.lib -Force

    if($build -eq "release")
    {
        return;
    }
    Move-Item ./a.pdb ./bin/$build/river2Dsoftware.pdb -Force
}

if($build -eq "release")
{
    compile ($args_always + $args_release)
}
elseif($build -eq "debug")
{
    compile ($args_always + $args_debug)
}
elseif($build -eq "asan")
{
    compile ($args_always + $args_asan)
}
else
{
    Write-Host "`nERROR: invalid make config: $build." -Fore Red
    exit 3;
}

Write-Host "`n"
