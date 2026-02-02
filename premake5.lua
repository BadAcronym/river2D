---@diagnostic disable: undefined-global, undefined-field
require"ecc/ecc"

-- TODAY: create separate compile targets for the renderer .dll/.so files
-- and the actual tile editor:
-- river2D_editor.exe   / river2D_editor
-- river2D_software.dll / libriver2D_software.so

workspace("river2D")
    configurations({ "debug", "release" })
    platforms({"windows", "linux"})
    location("build")
    architecture("x86_64")

project("river2D_editor")
    language("C++")
    cppdialect("C++23")
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
        targetdir("bin/Win64_%{cfg.buildcfg}")
        objdir("obj/Win64_%{cfg.buildcfg}")
        files({ "./src/win32*", "./include/win32*", "./src/river2D*", "./include/river2D*" })
        includedirs({ "./include/", "./vendor/stb/"})
        buildoptions{"/wd4068", "/wd4100"}
        ignoredefaultlibraries({ "MSVCRT" })

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
        includedirs({ "./include/", "/usr/include/", "./vendor/stb/"})
        linkoptions{"-lX11"}

project("river2D_software")
    language("C++")
    cppdialect("C++23")
    warnings("Extra")
    kind("SharedLib")
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
        targetdir("bin/Win64_%{cfg.buildcfg}")
        objdir("obj/Win64_%{cfg.buildcfg}")
        files({ "./src/win32*", "./include/win32*", "./src/river2D*", "./include/river2D*" })
        includedirs({ "./include/", "./vendor/stb/"})
        buildoptions{"/wd4068", "/wd4100"}
        ignoredefaultlibraries({ "MSVCRT" })

    filter("platforms:Linux")
        system("Linux")
        targetdir("bin/Linux_%{cfg.buildcfg}")
        objdir("obj/Linux_%{cfg.buildcfg}")
        files({ "./src/linux*", "./include/linux*", "./src/river2D*", "./include/river2D*" })
        includedirs({ "./include/", "/usr/include/", "./vendor/stb/"})
        linkoptions{"-lX11"}
