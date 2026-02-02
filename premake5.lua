---@diagnostic disable: undefined-global, undefined-field
require"ecc/ecc"

workspace("river2D")
    configurations({ "debug", "release" })
    platforms({"windows", "linux"})
    location("build")
    architecture("x86_64")

    project("river2D")
        language("C++")
        cppdialect("C++23")
        warnings("Extra")

filter("configurations:debug")
    defines{"DEBUG"}
    staticruntime("off")
    runtime("debug")
    symbols("On")
    ignoredefaultlibraries({ "MSVCRT" })

filter("configurations:release")
    staticruntime("off")
    runtime("release")
    symbols("Off")
    optimize("Speed")

filter("platforms:Windows")
    system("Windows")
    targetdir("bin/Win64_%{cfg.buildcfg}")
    objdir("obj/Win64_%{cfg.buildcfg}")
    files({ "./src/win32*", "./include/win32*", "./src/river2D*", "./include/river2D*" })
    includedirs({ "./include/" })
    buildoptions{"/wd4068", "/wd4100"}

filter({"platforms:Windows", "configurations:release"})
    kind("WindowedApp")

filter({"platforms:Windows", "configurations:debug"})
    kind("ConsoleApp")

filter("platforms:Linux")
    system("Linux")
    kind("ConsoleApp")
    targetdir("bin/Linux_%{cfg.buildcfg}")
    objdir("obj/Linux_%{cfg.buildcfg}")
    files({ "./src/linux*", "./include/linux*", "./src/river2D*", "./include/river2D*" })
    includedirs({ "./include/", "/usr/include/"})
    linkoptions{"-lX11"}
