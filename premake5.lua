---@diagnostic disable: undefined-global, undefined-field
require"vendor/premake-ecc/ecc"

workspace("river2D")
    configurations({ "debug", "release" })
    platforms({"linux", "windows"})
    location("build")
    architecture("x86_64")

project("river2D binary")
    language("C")
    cdialect("C99")
    warnings("Extra")
    targetname("river2Dmapedit")

    filter("configurations:debug")
        defines{"DEBUG"}
        runtime("debug")
        symbols("On")
        optimize("Off")

    filter("configurations:release")
        staticruntime("off")
        runtime("release")
        symbols("Off")
        optimize("Speed")

    filter("platforms:Linux")
        system("Linux")
        defines("BUILD_LINUX")
        kind("ConsoleApp")
        targetdir("bin/%{cfg.buildcfg}")
        objdir("obj/river2Dmapedit/")
        files({"./src/linux_river2Dmapedit*",
               "./include/linux_river2Dmapedit*",
               "./src/river2Dmapedit*",
               "./include/river2Dmapedit*" })
        includedirs({"./include/", "/usr/include/", "./vendor/imgsurf/include/"})
        libdirs({"./vendor/imgsurf/bin/%{cfg.buildcfg}/"})
        links("imgsurf:static")
        buildoptions({"-Wextra", "-Wall", "-Wpedantic"})
        linkoptions({"-lX11", "-lXrender", "-limgsurf", "-fuse-ld=mold"})
        toolset("clang")

    filter("platforms:Windows")
        system("Windows")
        defines("BUILD_WINDOWS")
        targetdir("bin/%{cfg.buildcfg}")
        objdir("obj/")
        files({ "./src/win32_river2Dmapedit*",
                "./include/win32_river2Dmapedit*",
                "./src/river2Dmapedit*",
                "./include/river2Dmapedit*" })
        includedirs({"./include/", "./vendor/imgsurf/include"})
        libdirs({"./vendor/imgsurf/bin/imgsurf_win64/%{cfg.buildcfg}/"})
        links({"imgsurf.lib"})
        buildoptions({"/wd4068"})

    filter({"platforms:Linux", "configurations:debug"})
        buildoptions({"-gfull", "-O0", "-fsanitize=address,leak,undefined", "-fno-omit-frame-pointer",
                      "-static-libasan"})
        linkoptions({"-gfull", "-O0", "-fsanitize=address,leak,undefined", "-fno-omit-frame-pointer",
                     "-static-libasan"})

    filter({"platforms:Windows", "configurations:debug"})
        kind("ConsoleApp")
        editandcontinue("Off")
        debugformat("c7")
        buildoptions({"/fsanitize=address"})

    filter({"platforms:Windows", "configurations:release"})
        kind("WindowedApp")
        linkoptions("/NODEFAULTLIB:MSVCRTD")

project("river2D common functions")
    language("C")
    cdialect("C99")
    warnings("Extra")
    kind("StaticLib")
    targetname("river2Dcommon")

    filter("configurations:debug")
        defines{"DEBUG"}
        runtime("debug")
        symbols("On")
        optimize("Off")

    filter("configurations:release")
        staticruntime("off")
        runtime("release")
        symbols("Off")
        optimize("Speed")

    filter("platforms:Linux")
        system("Linux")
        defines("BUILD_LINUX")
        targetdir("bin/%{cfg.buildcfg}")
        objdir("obj/river2Dcommon/")
        files({"./src/river2D_*",
               "./include/river2D_*",
               "./src/linux_river2Dcommon*",
               "./include/linux_river2Dcommon*",
               "./src/river2Dcommon*",
               "./include/river2Dcommon*"})
        includedirs({"./include/", "/usr/include/", "./vendor/imgsurf/include/"})
        libdirs({"./vendor/imgsurf/bin/%{cfg.buildcfg}/"})
        links("imgsurf:static")
        buildoptions({"-Wextra", "-Wall", "-Wpedantic"})
        linkoptions({"-lX11", "-lXrender", "-fuse-ld=mold"})
        toolset("clang")

    filter("platforms:Windows")
        system("Windows")
        defines("BUILD_WINDOWS")
        targetdir("bin/%{cfg.buildcfg}")
        objdir("obj/")
        files({"./src/river2D_*",
               "./include/river2D_*",
               "./src/win32_river2Dcommon*",
               "./include/win32_river2Dcommon*",
               "./src/river2Dcommon*",
               "./include/river2Dcommon*" })
        includedirs({"./include/", "./vendor/imgsurf/include"})
        libdirs({"./vendor/imgsurf/bin/%{cfg.buildcfg}/"})
        links({"imgsurf.lib"})
        buildoptions({"/wd4068"})

    filter({"platforms:Linux", "configurations:debug"})
        buildoptions({"-gfull", "-O0", "-fsanitize=address,leak,undefined", "-fno-omit-frame-pointer",
                      "-static-libasan"})
        linkoptions({"-gfull", "-O0", "-fsanitize=address,leak,undefined", "-fno-omit-frame-pointer",
                     "-static-libasan"})

    filter({"platforms:Windows", "configurations:debug"})
        editandcontinue("Off")
        debugformat("c7")
        buildoptions({"/fsanitize=address"})

    filter({"platforms:Windows", "configurations:release"})
        linkoptions("/NODEFAULTLIB:MSVCRTD")

project("river2D software renderer")
    language("C")
    cdialect("C99")
    warnings("Extra")
    kind("SharedLib")
    targetname("river2Dsoftware")

    filter("configurations:debug")
        defines{"DEBUG"}
        runtime("debug")
        symbols("On")
        optimize("Off")

    filter("configurations:release")
        staticruntime("off")
        runtime("release")
        symbols("Off")
        optimize("Speed")

    filter("platforms:Linux")
        system("Linux")
        defines("BUILD_LINUX")
        targetdir("bin/%{cfg.buildcfg}")
        objdir("obj/river2Dsoftware/")
        files({"./src/linux_river2Dsoftware*",
               "./include/linux_river2Dsoftware*",
               "./src/river2Dsoftware*",
               "./include/river2Dsoftware*" })
        includedirs({"./include/", "/usr/include/", "./vendor/imgsurf/include/"})
        libdirs({"./vendor/imgsurf/bin/%{cfg.buildcfg}/", "./bin/%{cfg.buildcfg}/"})
        links({"imgsurf:static", "river2Dcommon:static"})
        buildoptions({"-Wextra", "-Wall", "-Wpedantic"})
        linkoptions({"-lX11", "-lXrender", "-fuse-ld=mold", "-lriver2Dcommon"})
        toolset("clang")

    filter("platforms:Windows")
        system("Windows")
        defines("BUILD_WINDOWS")
        targetdir("bin/%{cfg.buildcfg}")
        objdir("obj/")
        files({"./src/win32_river2Dsoftware*",
               "./include/win32_river2Dsoftware*",
               "./src/river2Dsoftware*",
               "./include/river2Dsoftware*" })
        includedirs({"./include/", "./vendor/imgsurf/include"})
        libdirs({"./vendor/imgsurf/bin/%{cfg.buildcfg}/", "./bin/%{cfg.buildcfg}/"})
        links({"imgsurf.lib", "river2Dcommon.lib"})
        buildoptions({"/wd4068"})

    filter({"platforms:Linux", "configurations:debug"})
        buildoptions({"-gfull", "-O0", "-fsanitize=address,leak,undefined", "-fno-omit-frame-pointer",
                      "-static-libasan"})
        linkoptions({"-gfull", "-O0", "-fsanitize=address,leak,undefined", "-fno-omit-frame-pointer",
                     "-static-libasan"})

    filter({"platforms:Windows", "configurations:debug"})
        editandcontinue("Off")
        debugformat("c7")
        buildoptions({"/fsanitize=address"})

    filter({"platforms:Windows", "configurations:release"})
        linkoptions("/NODEFAULTLIB:MSVCRTD")
