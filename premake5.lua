---@diagnostic disable: undefined-global, undefined-field

workspace("river2D")
    configurations({"debug", "asan", "release"})
    platforms({"linux", "windows"})
    location("build")
    architecture("x86_64")

project("river2D_common")
    language("C")
    cdialect("C99")
    warnings("Extra")
    kind("StaticLib")
    targetname("river2Dcommon")
    libdirs({"./vendor/imgsurf/bin/%{cfg.buildcfg}/"})
    includedirs({"./include/",
                 "/usr/include/",
                 "./vendor/imgsurf/include/",
                 "./vendor/imgsurf/vendor/puddle/include/"})
    toolset("clang")
    buildoptions({"-Wextra", "-Wall", "-Wpedantic", "-Wconversion", "-Wshadow",
                  "-Wsign-compare", "-Wtype-limits", "-Wunused"})
    links("imgsurf:static")

    filter("configurations:asan")
        defines{"ASAN"}

    filter("configurations:debug")
        defines{"DEBUG"}

    filter("configurations:debug or asan")
        runtime("debug")
        symbols("On")
        optimize("Off")
        buildoptions({"-g", "-O0"});
        linkoptions({"-g", "-O0"});

    filter("configurations:release")
        staticruntime("off")
        runtime("release")
        symbols("Off")
        optimize("Speed")

    filter("platforms:linux")
        system("linux")
        defines("BUILD_LINUX")
        targetdir("bin/%{cfg.buildcfg}")
        objdir("obj/river2Dcommon/")
        files({"./src/river2D_*",
               "./include/river2D_*",
               "./src/linux_river2Dcommon*",
               "./include/linux_river2Dcommon*",
               "./src/river2Dcommon*",
               "./include/river2Dcommon*"})
        linkoptions({"-lX11", "-fuse-ld=mold"})

    filter("platforms:windows")
        system("windows")
        defines("BUILD_WINDOWS")
        targetdir("bin/%{cfg.buildcfg}")
        objdir("obj/")
        files({"./src/river2D_*",
               "./include/river2D_*",
               "./src/win32_river2Dcommon*",
               "./include/win32_river2Dcommon*",
               "./src/river2Dcommon*",
               "./include/river2Dcommon*"})

    filter({"platforms:linux", "configurations:asan"})
        buildoptions({"-fsanitize=address,leak,undefined", "-fno-omit-frame-pointer",
                      "-static-libasan"})
        linkoptions({"-fsanitize=address,leak,undefined", "-fno-omit-frame-pointer",
                     "-static-libasan"})

    filter({"platforms:linux", "configurations:debug or asan"})
        buildoptions("-gfull");
        linkoptions("-gfull");

    filter({"platforms:windows", "configurations:debug or asan"})
        buildoptions("-gcodeview");
        linkoptions("-gcodeview");

    filter({"platforms:windows", "configurations:asan"})
        toolset("clang-cl")
        buildoptions({"/fsanitize=address", "/Zi", "/INCREMENTAL:NO"})
        linkoptions{"/link clang_rt.asan_dynamic-x86_64.lib clang_rt.asan_dynamic_runtime_thunk-x86_64.lib"}
        editandcontinue("Off")

project("river2D_software")
    language("C")
    cdialect("C99")
    warnings("Extra")
    kind("SharedLib")
    targetname("river2Dsoftware")
    libdirs({"./vendor/imgsurf/bin/%{cfg.buildcfg}/", "./bin/%{cfg.buildcfg}/"})
    includedirs({"./include/",
                 "/usr/include/",
                 "./vendor/imgsurf/include/",
                 "./vendor/imgsurf/vendor/puddle/include/"})
    toolset("clang")
    buildoptions({"-Wextra", "-Wall", "-Wpedantic", "-Wconversion", "-Wshadow",
                  "-Wsign-compare", "-Wtype-limits"})
    links({"imgsurf:static", "river2Dcommon:static"})

    filter("configurations:asan")
        defines{"ASAN"}

    filter("configurations:debug")
        defines{"DEBUG"}

    filter("configurations:debug or asan")
        runtime("debug")
        symbols("On")
        optimize("Off")
        buildoptions({"-g", "-O0"});
        linkoptions({"-g", "-O0"});

    filter("configurations:release")
        staticruntime("off")
        runtime("release")
        symbols("Off")
        optimize("Speed")

    filter("platforms:linux")
        system("linux")
        defines("BUILD_LINUX")
        targetdir("bin/%{cfg.buildcfg}")
        objdir("obj/river2Dsoftware/")
        files({"./src/linux_river2Dsoftware*",
               "./include/linux_river2Dsoftware*",
               "./src/river2Dsoftware*",
               "./include/river2Dsoftware*"})
        linkoptions({"-lX11", "-lXrender", "-lriver2Dcommon", "-lm", "-fuse-ld=mold"})

    filter("platforms:windows")
        system("windows")
        defines("BUILD_WINDOWS")
        targetdir("bin/%{cfg.buildcfg}")
        objdir("obj/")
        files({"./src/win32_river2Dsoftware*",
               "./include/win32_river2Dsoftware*",
               "./src/river2Dsoftware*",
               "./include/river2Dsoftware*"})
        linkoptions({"-lriver2Dcommon", "-lgdi32", "-luser32"})

    filter({"platforms:linux", "configurations:asan"})
        buildoptions({"-fsanitize=address,leak,undefined", "-fno-omit-frame-pointer",
                      "-static-libasan"})
        linkoptions({"-fsanitize=address,leak,undefined", "-fno-omit-frame-pointer",
                     "-static-libasan"})

    filter({"platforms:linux", "configurations:debug or asan"})
        buildoptions("-gfull");
        linkoptions("-gfull");

    filter({"platforms:windows", "configurations:debug or asan"})
        buildoptions("-gcodeview");
        linkoptions("-gcodeview");

    filter({"platforms:windows", "configurations:asan"})
        toolset("clang-cl")
        buildoptions({"/fsanitize=address", "/Zi", "/INCREMENTAL:NO"})
        linkoptions{"/link clang_rt.asan_dynamic-x86_64.lib clang_rt.asan_dynamic_runtime_thunk-x86_64.lib"}
        editandcontinue("Off")
