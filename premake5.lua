---@diagnostic disable: undefined-global, undefined-field
require"ecc/ecc"

workspace("river2D")
    configurations({ "Debug", "Release" })
    platforms({"Windows", "Linux"})
    location("build")
    architecture("x86_64")

    project("river2D")
        language("C")
        cdialect("C23")
        warnings("Extra")
        includedirs({ "./include/", })
        files({ "./src/*", "./include/*" })
        -- buildoptions{"/wd4068", "/wd4100"}

filter("configurations:Debug")
    defines{"DEBUG"}
    staticruntime("off")
    runtime("Debug")
    symbols("On")
    ignoredefaultlibraries({ "MSVCRT" })

filter("configurations:Release")
    staticruntime("off")
    runtime("Release")
    symbols("Off")
    optimize("Speed")

filter("platforms:Windows")
    system("Windows")
    targetdir("bin/Win64_%{cfg.buildcfg}")
    objdir("obj/Win64_%{cfg.buildcfg}")

filter({"platforms:Windows", "configurations:Release"})
    kind("WindowedApp")

filter({"platforms:Windows", "configurations:Debug"})
    kind("ConsoleApp")

filter("platforms:Linux")
    system("Linux")
    kind("ConsoleApp")
    targetdir("bin/Linux_%{cfg.buildcfg}")
    objdir("obj/Linux_%{cfg.buildcfg}")
