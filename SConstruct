import subprocess
import os.path

build = ARGUMENTS.get("build", "release")
target = ARGUMENTS.get("target", "all")

project_name = "Melodia"
cpp_standard = "17"
sfml_dir = "external/build/SFML"
build_jobs = 4
use_system_sfml = (target == "linux")

sources = [
    "src/main.cpp",
    "src/data.cpp",
    "src/menus.cpp",
    "src/player_menu.cpp",
    "src/playlist_selector_menu.cpp",
    "src/animation.cpp",
    "external/lib/RoundedRectangleShape.cpp"
]

base = Environment(
    CPPPATH=["include", "external/lib"],
    CPPDEFINES=["CPPHTTPLIB_OPENSSL_SUPPORT"],
    CXXFLAGS=[f"-std=c++{cpp_standard}"],
    LIBS=["libssl", "libcrypto"],
)

if not use_system_sfml:
    base.Append(CPPPATH=[f"{sfml_dir}/include"])

no_extras_in_build = [
    "-DBUILD_SHARED_LIBS=FALSE",
    "-DBUILD_TESTING=OFF"
]

def build_target(env, platform):
    build_dir = f"build/{platform}"

    VariantDir(build_dir, ".", duplicate=0)

    return env.Program(
        target=f"{out_dir}/{platform}/{project_name + '.exe' if platform == 'windows' else project_name}",
        source=[f"{build_dir}/{src}" for src in sources],
    )

def ensure_sfml_repo():
    if not os.path.exists(sfml_dir):
        print("Cloning SFML...")
        subprocess.check_call([
            "git", "clone", "--depth", "1",
            "https://github.com/SFML/SFML.git",
            sfml_dir
        ])

def ensure_dependency_build(dep_name, source_dir, build_dir, cmake_args):
    cmake_args += no_extras_in_build

    lib_path = os.path.join(build_dir, "lib")

    if os.path.exists(lib_path) and any(name.startswith(f"lib{dep_name}") for name in os.listdir(lib_path)):
        print(f"{dep_name} already built in {build_dir}")
        return

    print(f"Building {dep_name} in {build_dir}...")

    os.makedirs(build_dir, exist_ok=True)

    subprocess.check_call(
        ["cmake", os.path.abspath(source_dir)] + cmake_args,
        cwd=build_dir
    )

    subprocess.check_call(
        ["cmake", "--build", ".", f"-j{build_jobs}"],
        cwd=build_dir
    )

    print(f"{dep_name} built in {build_dir}")

def ensure_deps_repo(dep_name, repo_url, branch = None):
    dep_dir = f"external/build/{dep_name}"
    if not os.path.exists(dep_dir):
        print(f"Cloning {dep_name}...")
        subprocess.check_call([
            *f"git clone --depth 1{(' --branch ' + branch) if branch else ''}".split(' '),
            repo_url,
            dep_dir
        ])


def ensure_sfml_build(build_dir, cmake_args):
    lib_path = os.path.join(build_dir, "lib")

    if os.path.exists(lib_path) and any(name.startswith("libsfml") for name in os.listdir(lib_path)):
        print(f"SFML already built in {build_dir}")
        return

    print(f"Building SFML in {build_dir}...")

    os.makedirs(build_dir, exist_ok=True)

    subprocess.check_call(
        ["cmake", ".."] + cmake_args,
        cwd=build_dir
    )

    subprocess.check_call(
        ["cmake", "--build", ".", f"-j{build_jobs}"],
        cwd=build_dir
    )

    print(f"SFML built in {build_dir}")

def ensure_openssl_build(source_dir, build_dir, install_dir):
    if os.path.exists(os.path.join(install_dir, "include", "openssl")):
        print(f"OpenSSL already built in {build_dir}")
        return

    print("Building OpenSSL for Windows (MinGW cross-compile)...")
    os.makedirs(build_dir, exist_ok=True)

    subprocess.check_call([
        "perl", os.path.abspath(f"{source_dir}/Configure"),
        "mingw64",
        "no-shared",
        "no-tests",
        f"--cross-compile-prefix=x86_64-w64-mingw32-",
        f"--prefix={os.path.abspath(install_dir)}",
        f"--openssldir={os.path.abspath(install_dir)}",
    ], cwd=build_dir)

    subprocess.check_call(["make", f"-j{build_jobs}"], cwd=build_dir)
    subprocess.check_call(["make", "install_sw"], cwd=build_dir)

    print(f"OpenSSL built in {build_dir}")

if "debug" in build: # Same as debug-l0
    base.Append(CXXFLAGS=[
        "-g",
        "-O0",
        "-Wall",
        "-Wextra"
    ])

    out_dir = "bin/debug"
else:
    base.Append(CXXFLAGS=["-O2"])
    out_dir = "bin/release"

if build == "debug-l1":
    base.Append(CXXFLAGS=[
        "-fsanitize=address",
        "-fsanitize=undefined",
        "-fno-omit-frame-pointer",
        "-fno-sanitize-recover=all"
    ])

    base.Append(LINKFLAGS=[
        "-fsanitize=address",
        "-fsanitize=undefined"
    ])

targets = []

if target in ("all", "linux"):
    print("Building Linux")

    env = base.Clone()

    env.ParseConfig("pkg-config --cflags --libs sfml-graphics sfml-audio")

    prog = build_target(env, "linux")
    targets.append(prog)

if target in ("all", "windows"):
    print("Building Windows")

    ensure_sfml_repo()

    ensure_deps_repo("libogg", "https://github.com/xiph/ogg.git")
    ensure_deps_repo("libvorbis", "https://github.com/xiph/vorbis.git")
    ensure_deps_repo("flac", "https://github.com/xiph/flac.git")
    ensure_deps_repo("openal-soft", "https://github.com/kcat/openal-soft.git")
    ensure_deps_repo("freetype", "https://github.com/freetype/freetype.git")
    ensure_deps_repo("harfbuzz", "https://github.com/harfbuzz/harfbuzz.git")
    ensure_deps_repo("openssl", "https://github.com/openssl/openssl.git")

    deps_build_dir = "external/build/build-mingw-deps"

    ensure_dependency_build("ogg", "external/build/libogg", f"{deps_build_dir}/ogg", [
        "-DCMAKE_SYSTEM_NAME=Windows",
        "-DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc",
        "-DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++",
    ])

    ensure_dependency_build("vorbis", "external/build/libvorbis", f"{deps_build_dir}/vorbis", [
        "-DCMAKE_SYSTEM_NAME=Windows",
        "-DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc",
        "-DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++",
        "-DBUILD_TESTING=OFF",
        f"-DOGG_INCLUDE_DIR={os.path.abspath('external/build/libogg/include')}",
        f"-DOGG_LIBRARY={os.path.abspath(deps_build_dir)}/ogg/lib/libogg.a",
    ])

    ensure_dependency_build("flac", "external/build/flac", f"{deps_build_dir}/flac", [
        "-DCMAKE_SYSTEM_NAME=Windows",
        "-DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc",
        "-DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++",
        "-DINSTALL_MANPAGES=OFF",
        "-DBUILD_PROGRAMS=OFF",
        "-DBUILD_EXAMPLES=OFF",
        "-DBUILD_DOCS=OFF",
        f"-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY={os.path.abspath(deps_build_dir)}/flac/lib",
        f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={os.path.abspath(deps_build_dir)}/flac/lib",
        f"-DCMAKE_INSTALL_PREFIX={os.path.abspath(deps_build_dir)}/flac/install",
        f"-DOGG_INCLUDE_DIR={os.path.abspath('external/build/libogg/include')}",
        f"-DOGG_LIBRARY={os.path.abspath(deps_build_dir)}/ogg/lib/libogg.a",
    ])

    ensure_dependency_build("openal-soft", "external/build/openal-soft", f"{deps_build_dir}/openal-soft", [
        "-DCMAKE_SYSTEM_NAME=Windows",
        "-DBUILD_SHARED_LIBS=FALSE",
        "-DALSOFT_EXAMPLES=OFF",
        "-DALSOFT_UTILS=OFF",
        "-DALSOFT_TESTS=OFF",
        "-DALSOFT_BACKEND_PIPEWIRE=OFF",
        "-DALSOFT_BACKEND_PULSEAUDIO=OFF",
        "-DALSOFT_BACKEND_ALSA=OFF",
        "-DALSOFT_BACKEND_OSS=OFF",
        "-DALSOFT_BACKEND_SNDIO=OFF",
        "-DALSOFT_INSTALL_CONFIG=OFF"
        "-DALSOFT_REQUIRE_WINMM=ON",
        "-DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc",
        "-DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++",
    ])

    ensure_dependency_build("harfbuzz", "external/build/harfbuzz", f"{deps_build_dir}/harfbuzz", [
        "-DCMAKE_SYSTEM_NAME=Windows",
        "-DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc",
        "-DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++",
        "-DBUILD_SHARED_LIBS=FALSE",
    ])

    ensure_dependency_build("freetype", "external/build/freetype", f"{deps_build_dir}/freetype", [
        "-DCMAKE_SYSTEM_NAME=Windows",
        "-DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc",
        "-DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++",
        "-DBUILD_SHARED_LIBS=FALSE",
    ])

    ensure_openssl_build(
        "external/build/openssl",
        f"{deps_build_dir}/openssl/build",
        f"{deps_build_dir}/openssl/install"
    )

    build_dir = f"{sfml_dir}/build-mingw"

    ensure_sfml_build(build_dir, [
        "-DCMAKE_SYSTEM_NAME=Windows",
        "-DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc",
        "-DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++",
        "-DBUILD_SHARED_LIBS=FALSE",
    ])

    env = base.Clone()

    env.Replace(
        CXX="x86_64-w64-mingw32-g++",
        SHLINK="x86_64-w64-mingw32-g++",
    )

    env.Append(
        CXXFLAGS=[
            "-fpermissive"
        ],
        LINKFLAGS=[
            "-static-libgcc",
            "-static-libstdc++",
            "-static"
        ],
        LIBPATH=[
            f"{build_dir}/lib",
            f"{deps_build_dir}/ogg/lib",
            f"{deps_build_dir}/vorbis/lib",
            f"{deps_build_dir}/flac/lib",
            f"{deps_build_dir}/openal-soft",
            f"{deps_build_dir}/harfbuzz",
            f"{deps_build_dir}/freetype/lib",
            f"{deps_build_dir}/openssl/install/lib64"
        ],
        CPPPATH=[
            "include",
            f"{sfml_dir}/include",
            f"{deps_build_dir}/ogg/include",
            f"{deps_build_dir}/vorbis/include",
            f"{deps_build_dir}/flac/include",
            f"{deps_build_dir}/openal-soft/include",
            f"{deps_build_dir}/harfbuzz/include",
            f"{deps_build_dir}/freetype/include",
            f"{deps_build_dir}/openssl/install/include"
        ],
        LIBS=[
            "sfml-graphics-s",
            "sfml-audio-s",
            "sfml-window-s",
            "sfml-system-s",
            "ogg",
            "vorbis",
            "vorbisfile",
            "FLAC",
            "OpenAL32",
            "opengl32",
            "winmm",
            "gdi32",
            "harfbuzz",
            "freetype",
            "crypt32",
            "ws2_32"
        ],
        CPPDEFINES=["SFML_STATIC"]
    )

    prog = build_target(env, "windows")
    targets.append(prog)

if target in ("all", "macos"):
    print("Building macOS")

    ensure_sfml_repo()

    build_dir = f"{sfml_dir}/build-macos"

    ensure_sfml_build(build_dir, [
        "-DCMAKE_BUILD_TYPE=Release",
        "-DBUILD_SHARED_LIBS=FALSE",
    ])

    env = base.Clone()

    env.Replace(CXX="clang++")

    env.Append(
        LIBPATH=[f"{build_dir}/lib"],
        LIBS=[
            "sfml-graphics",
            "sfml-audio",
            "sfml-window",
            "sfml-system",
        ],
    )

    prog = build_target(env, "macos")
    targets.append(prog)

Default(targets)
