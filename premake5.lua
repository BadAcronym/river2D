---@diagnostic disable: undefined-global, undefined-field
require"vendor/premake-ecc/ecc"

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
        runtime("debug")
        symbols("On")
        optimize("Off")

    filter("configurations:release")
        staticruntime("off")
        runtime("release")
        symbols("Off")
        optimize("Speed")

    filter("platforms:Windows")
        system("Windows")
        defines("BUILD_WINDOWS")
        targetdir("bin/river2Dmapedit_win64/%{cfg.buildcfg}")
        objdir("obj/river2Dmapedit/%{cfg.buildcfg}")
        files({ "./src/win32_river2D*",
                "./include/win32_river2D*",
                "./src/river2D*",
                "./include/river2D*" })
        includedirs({"./include/", "./vendor/imgsurf/include"})
        libdirs({"./vendor/imgsurf/bin/**",
            "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.26100.0/km/x64/"})
        links({"imgsurf.lib", "ntoskrnl.lib"})
        ignoredefaultlibraries({ "MSVCRT" })

    filter({"platforms:Windows", "configurations:release"})
        kind("WindowedApp")

    filter({"platforms:Windows", "configurations:debug"})
        kind("ConsoleApp")

    filter("platforms:Linux")
        system("Linux")
        defines("BUILD_LINUX")
        kind("ConsoleApp")
        targetdir("bin/river2Dmapedit_linux/%{cfg.buildcfg}")
        objdir("obj/river2Dmapedit/%{cfg.buildcfg}")
        files({"./src/linux_river2D*",
               "./include/linux_river2D*",
               "./src/river2D*",
               "./include/river2D*" })
        includedirs({"./include/", "/usr/include/", "./vendor/imgsurf/include/"})
        libdirs({"./vendor/imgsurf/bin/**"})
        links("imgsurf:static")
        buildoptions({"-Wextra", "-Wall", "-Werror"})
        linkoptions({"-lX11", "-lXrender", "-fuse-ld=mold"})
        toolset("clang")

    filter({"platforms:Linux", "configurations:debug"})
        buildoptions({"-gfull", "-O0", "-fsanitize=address,leak,undefined", "-fno-omit-frame-pointer",
                      "-static-libasan"})
        linkoptions({"-gfull", "-O0", "-fsanitize=address,leak,undefined", "-fno-omit-frame-pointer",
                     "-static-libasan"})

project("river2D common functions")
    language("C")
    cdialect("C23")
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

    filter("platforms:Windows")
        system("Windows")
        defines("BUILD_WINDOWS")
        targetdir("bin/river2Dcommon_win64/%{cfg.buildcfg}")
        objdir("obj/river2Dcommon/%{cfg.buildcfg}")
        files({"./src/river2D_*",
               "./include/river2D_*",
               "./src/win32_river2Dcommon*",
               "./include/win32_river2Dcommon*",
               "./src/river2Dcommon*",
               "./include/river2Dcommon*" })
        includedirs({"./include/", "./vendor/imgsurf/include"})
        libdirs({"./vendor/imgsurf/bin/**",
            "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.26100.0/km/x64/"})
        links({"imgsurf.lib", "ntoskrnl.lib"})
        ignoredefaultlibraries({ "MSVCRT" })

    filter("platforms:Linux")
        system("Linux")
        defines("BUILD_LINUX")
        targetdir("bin/river2Dcommon_linux/%{cfg.buildcfg}")
        objdir("obj/river2Dcommon/%{cfg.buildcfg}")
        files({"./src/river2D_*",
               "./include/river2D_*",
               "./src/linux_river2Dcommon*",
               "./include/linux_river2Dcommon*",
               "./src/river2Dcommon*",
               "./include/river2Dcommon*" })
        includedirs({"./include/", "/usr/include/", "./vendor/imgsurf/include/"})
        libdirs({"./vendor/imgsurf/bin/**"})
        links("imgsurf:static")
        buildoptions({"-Wextra", "-Wall", "-Werror"})
        linkoptions({"-lX11", "-lXrender", "-fuse-ld=mold"})
        toolset("clang")

    filter({"platforms:Linux", "configurations:debug"})
        buildoptions({"-gfull", "-O0", "-fsanitize=address,leak,undefined", "-fno-omit-frame-pointer",
                      "-static-libasan"})
        linkoptions({"-gfull", "-O0", "-fsanitize=address,leak,undefined", "-fno-omit-frame-pointer",
                     "-static-libasan"})

project("river2D software renderer")
    language("C")
    cdialect("C23")
    warnings("Extra")
    kind("StaticLib")
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

    filter("platforms:Windows")
        system("Windows")
        defines("BUILD_WINDOWS")
        targetdir("bin/river2Dsoftware_win64/%{cfg.buildcfg}")
        objdir("obj/river2Dsoftware/%{cfg.buildcfg}")
        files({"./src/river2D_*",
               "./include/river2D_*",
               "./src/win32_river2Dsoftware*",
               "./include/win32_river2Dsoftware*",
               "./src/river2Dsoftware*",
               "./include/river2Dsoftware*" })
        includedirs({"./include/", "./vendor/imgsurf/include"})
        libdirs({"./vendor/imgsurf/bin/**"})
        links("imgsurf:static")
        ignoredefaultlibraries({ "MSVCRT" })

    filter("platforms:Linux")
        system("Linux")
        defines("BUILD_LINUX")
        targetdir("bin/river2Dsoftware_linux/%{cfg.buildcfg}")
        objdir("obj/river2Dsoftware/%{cfg.buildcfg}")
        files({"./src/river2D_*",
               "./include/river2D_*",
               "./src/linux_river2Dsoftware*",
               "./include/linux_river2Dsoftware*",
               "./src/river2Dsoftware*",
               "./include/river2Dsoftware*" })
        includedirs({"./include/", "/usr/include/", "./vendor/imgsurf/include/"})
        libdirs({"./vendor/imgsurf/bin/**"})
        links("imgsurf:static")
        buildoptions({"-Wextra", "-Wall", "-Werror"})
        linkoptions({"-lX11", "-lXrender", "-fuse-ld=mold"})
        toolset("clang")

    filter({"platforms:Linux", "configurations:debug"})
        buildoptions({"-gfull", "-O0", "-fsanitize=address,leak,undefined", "-fno-omit-frame-pointer",
                      "-static-libasan"})
        linkoptions({"-gfull", "-O0", "-fsanitize=address,leak,undefined", "-fno-omit-frame-pointer",
                     "-static-libasan"})
