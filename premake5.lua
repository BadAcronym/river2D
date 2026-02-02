---@diagnostic disable: undefined-global, undefined-field
require"vendor/premake-ecc/ecc"

-- TODAY: create separate compile targets for the renderer .dll/.so files
-- and the actual tile editor:
-- river2D_editor.exe   / river2D_editor
-- river2D_software.dll / libriver2D_software.so

workspace("river2D")
    configurations({ "debug", "release" })
    platforms({"linux", "windows"})
    location("build")
    architecture("x86_64")

project("river2D binary")
    language("C")
    cdialect("C23")
    warnings("Extra")
    targetname("river2Dmapedit")

    filter("configurations:debug")
        defines{"DEBUG"}
        staticruntime("off")
        runtime("debug")
        symbols("On")

    filter("configurations:release")
        staticruntime("off")
        runtime("release")
        symbols("Off")
        optimize("Speed")

    filter("platforms:Windows")
        system("Windows")
        defines("BUILD_WINDOWS")
        targetdir("bin/Win64_%{cfg.buildcfg}")
        objdir("obj/Win64_%{cfg.buildcfg}")
        files({
            "./src/win32_river2D*",
            "./include/win32_river2D*",
            "./src/river2D*",
            "./include/river2D*" })
        includedirs({ "./include/", "./vendor/stb/"})
        buildoptions{"/wd4068", "/wd4100"}
        ignoredefaultlibraries({ "MSVCRT" })

    filter({"platforms:Windows", "configurations:release"})
        kind("WindowedApp")

    filter({"platforms:Windows", "configurations:debug"})
        kind("ConsoleApp")

    filter("platforms:Linux")
        system("Linux")
        defines("BUILD_LINUX")
        kind("ConsoleApp")
        targetdir("bin/Linux_%{cfg.buildcfg}")
        objdir("obj/")
        files({
            "./src/linux_river2D*",
            "./include/linux_river2D*",
            "./src/river2D*",
            "./include/river2D*" })
        includedirs({ "./include/", "/usr/include/", "./vendor/stb/"})
        linkoptions{"-lX11", "-fuse-ld=mold"}

project("river2D software renderer")
    language("C")
    cdialect("C23")
    warnings("Extra")
    kind("StaticLib")
    targetname("river2Dsoftware")

    filter("configurations:debug")
        defines{"DEBUG"}
        staticruntime("off")
        runtime("debug")
        symbols("On")

    filter("configurations:release")
        staticruntime("off")
        runtime("release")
        symbols("Off")
        optimize("Speed")

    filter("platforms:Windows")
        system("Windows")
        defines("BUILD_WINDOWS")
        targetdir("bin/Win64_%{cfg.buildcfg}")
        objdir("obj/Win64_%{cfg.buildcfg}")
        files({
            "./src/river2D_main.c",
            "./include/river2D_main.h",
            "./src/win32_river2Dsoftware*",
            "./include/win32_river2Dsoftware*",
            "./src/river2Dsoftware*",
            "./include/river2Dsoftware*" })
        includedirs({ "./include/", "./vendor/stb/"})
        buildoptions{"/wd4068", "/wd4100"}
        ignoredefaultlibraries({ "MSVCRT" })

    filter("platforms:Linux")
        system("Linux")
        defines("BUILD_LINUX")
        targetdir("bin/Linux_%{cfg.buildcfg}")
        objdir("obj/Linux_")
        files({
            "./src/river2D_main.c",
            "./include/river2D_main.h",
            "./src/linux_river2Dsoftware*",
            "./include/linux_river2Dsoftware*",
            "./src/river2Dsoftware*",
            "./include/river2Dsoftware*" })
        includedirs({ "./include/", "/usr/include/", "./vendor/stb/"})
        linkoptions{"-lX11"}
