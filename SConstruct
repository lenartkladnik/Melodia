project_name = "Melodia"
cpp_standard = "17"

build_type = ARGUMENTS.get("build", "all")

sources = [
    "src/main.cpp",
    "include/data.cpp",
    "include/menus.cpp",
    "include/player_menu.cpp",
    "include/playlist_selector_menu.cpp",
    "include/RoundedRectangleShape.cpp",
    "include/animation.cpp",
]

if build_type == "debug":
    print("Building debug...")

    env = Environment(
        CPPPATH=["include"]
    )
    env.ParseConfig("pkg-config --cflags --libs sfml-graphics sfml-audio")
    env.Append(
        CXXFLAGS=[f"-std=c++{cpp_standard}", "-g", "-O0", "-Wall", "-Wextra"],
    )

    executable = env.Program(
        target="bin/debug/linux/" + project_name,
        source=sources,
    )

    Default(executable)

targets = []

if build_type == "all" or build_type == "linux":
    print("Compiling for linux...")

    env = Environment(
        CPPPATH=["include"]
    )
    env.ParseConfig("pkg-config --cflags --libs sfml-graphics sfml-audio")
    env.Append(
        CXXFLAGS=[f"-std=c++{cpp_standard}", "-O2"],
    )

    linux_prog = env.Program(
        target="bin/release/linux/" + project_name,
        source=sources,
    )
    targets.append(linux_prog)

if build_type == "all" or build_type == "windows":
    print("Compiling for windows...")
    env = Environment(
        CXX="x86_64-w64-mingw32-g++",
        CPPPATH=["include", "/usr/x86_64-w64-mingw32/include"],
        LIBPATH=["/usr/x86_64-w64-mingw32/lib"],
        LIBS=["sfml-graphics", "sfml-audio"],
        CXXFLAGS=[f"-std=c++{cpp_standard}", "-O2"],
    )

    windows_prog = env.Program(
        target="bin/release/windows/" + project_name + ".exe",
        source=sources,
    )
    targets.append(windows_prog)

if build_type == "all" or build_type == "macos":
    env = Environment(
        CXX="clang++",
        CPPPATH=["include", "/opt/osxcross/SDK/MacOSX.sdk/usr/include"],
        LIBPATH=["/opt/osxcross/SDK/MacOSX.sdk/usr/lib"],
        LIBS=["sfml-graphics", "sfml-audio"],
        CXXFLAGS=[f"-std=c++{cpp_standard}", "-O2", "-fPIC"],
    )

    macos_prog = env.Program(
        target="bin/release/macos/" + project_name,
        source=sources,
    )
    targets.append(macos_prog)

if targets:
    Default(targets)
