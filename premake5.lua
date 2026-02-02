---@diagnostic disable: undefined-global, undefined-field

workspace("river2D")
    configurations({"debug", "asan", "release"})
    platforms({"linux", "windows"})
    location("build")
    architecture("x86_64")

project("river2D common functions")
    language("C")
    cdialect("C99")
    warnings("Extra")
    kind("StaticLib")
    targetname("river2Dcommon")
    libdirs({"./vendor/imgsurf/bin/%{cfg.buildcfg}/"})
    includedirs({"./include/", "/usr/include/", "./vendor/imgsurf/include/"})

    filter("configurations:asan")
        defines{"ASAN"}

    filter("configurations:debug")
        defines{"DEBUG"}

    filter("configurations:debug or asan")
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
        links("imgsurf:static")
        buildoptions({"-Wextra", "-Wall", "-Wpedantic"})
        linkoptions({"-lX11", "-fuse-ld=mold"})
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
        links({"imgsurf.lib"})
        buildoptions({"/wd4068"})

    filter({"platforms:Linux", "configurations:debug or asan"})
        buildoptions({"-gfull", "-O1"})
        linkoptions({"-gfull", "-O1"})

    filter({"platforms:Linux", "configurations:asan"})
        buildoptions({"-fsanitize=address,leak,undefined", "-fno-omit-frame-pointer",
                      "-static-libasan"})
        linkoptions({"-fsanitize=address,leak,undefined", "-fno-omit-frame-pointer",
                     "-static-libasan"})

    filter({"platforms:Windows", "configurations:asan"})
        editandcontinue("Off")
        buildoptions({"/fsanitize=address", "/Zi", "/INCREMENTAL:NO"})

    filter({"platforms:Windows", "configurations:release"})
        linkoptions("/NODEFAULTLIB:MSVCRTD")

project("river2D software renderer")
    language("C")
    cdialect("C99")
    warnings("Extra")
    kind("SharedLib")
    targetname("river2Dsoftware")
    libdirs({"./vendor/imgsurf/bin/%{cfg.buildcfg}/", "./bin/%{cfg.buildcfg}/"})
    includedirs({"./include/", "/usr/include/", "./vendor/imgsurf/include/"})

    filter("configurations:asan")
        defines{"ASAN"}

    filter("configurations:debug")
        defines{"DEBUG"}

    filter("configurations:debug or asan")
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
        links({"imgsurf:static", "river2Dcommon:static"})
        buildoptions({"-Wextra", "-Wall", "-Wpedantic"})
        linkoptions({"-lX11", "-lXrender", "-lriver2Dcommon", "-lm", "-fuse-ld=mold"})
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
        links({"imgsurf.lib", "river2Dcommon.lib"})
        buildoptions({"/wd4068"})

    filter({"platforms:Linux", "configurations:debug or asan"})
        buildoptions({"-gfull", "-O1"})
        linkoptions({"-gfull", "-O1"})

    filter({"platforms:Linux", "configurations:asan"})
        buildoptions({"-fsanitize=address,leak,undefined", "-fno-omit-frame-pointer",
                      "-static-libasan"})
        linkoptions({"-fsanitize=address,leak,undefined", "-fno-omit-frame-pointer",
                     "-static-libasan"})

    filter({"platforms:Windows", "configurations:asan"})
        editandcontinue("Off")
        buildoptions({"/fsanitize=address", "/Zi", "/INCREMENTAL:NO"})

    filter({"platforms:Windows", "configurations:release"})
        linkoptions("/NODEFAULTLIB:MSVCRTD")
