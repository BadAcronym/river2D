---@diagnostic disable: undefined-global, undefined-field
require"ecc/ecc"

workspace("river2D")
    configurations({ "Debug", "Release" })
    location("build")
    system("Windows")
    architecture("x86_64")

    project("river2D")
        language("C")
        cdialect("C23")
        warnings("Extra")
        targetdir("bin/Win64_%{cfg.buildcfg}")
        objdir("obj/Win64_%{cfg.buildcfg}")
        includedirs({ "./include/", })
        files({ "./src/*", "./include/*" })
        -- buildoptions{"/wd4068", "/wd4100"}

filter("configurations:Debug")
    kind("ConsoleApp")
    defines{"DEBUG"}
    staticruntime("off")
    runtime("Debug")
    symbols("On")
    ignoredefaultlibraries({ "MSVCRT" })

filter("configurations:Release")
    kind("WindowedApp")
    staticruntime("off")
    runtime("Release")
    symbols("Off")
    optimize("Speed")
