# How to Build (Qt 6.8.3) - Ubuntu 24.04 aarch64 (ARM64)

This covers building on **Linux ARM64/aarch64**, as opposed to x86\_64 Ubuntu (see `Build-Ubuntu-Qt6.8.2.md`) or macOS (see `CompilingForMac.md`).

Ubuntu's own Qt6 packages are far too old (Qt 6.4 on 24.04), and Qt does not publish prebuilt Linux ARM64 packages through its online installer, so Qt has to be built from source. Everything else (OpenCV, Tesseract, D-Bus) is available through `apt` on Ubuntu 24.04.

This was verified with:
- Ubuntu 24.04.4 LTS, kernel 6.17, aarch64
- Qt 6.8.3 (built from source)
- CMake 3.28, Ninja, GCC (system default)

## 1. Install build tools and dependencies

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build git pkg-config

# Libraries this project links against directly
sudo apt install libopencv-dev libtesseract-dev libleptonica-dev \
    libdbus-1-dev libsdbus-c++-dev libopenexr-dev \
    libgl1-mesa-dev libglx-dev libglu1-mesa-dev

# Dependencies needed to build Qt itself (xcb platform plugin, fonts, etc.)
sudo apt install libxkbcommon-dev libxkbcommon-x11-dev libxcb-cursor-dev \
    libxcb-icccm4-dev libxcb-image0-dev libxcb-keysyms1-dev libxcb-randr0-dev \
    libxcb-render-util0-dev libxcb-shape0-dev libxcb-sync-dev libxcb-xfixes0-dev \
    libxcb-xinerama0-dev libxcb-xkb-dev libxcb1-dev libx11-dev libx11-xcb-dev \
    libxext-dev libxfixes-dev libxi-dev libxrender-dev libfontconfig1-dev \
    libfreetype-dev libssl-dev

# FFmpeg dev libs so Qt Multimedia gets a working webcam/video backend
sudo apt install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev \
    libswresample-dev
```

Note: `libopencv-dev` on 24.04 provides OpenCV 4.6, and `libtesseract-dev` provides Tesseract 5.3.4. Both are picked up via `pkg-config` (`opencv4`, `tesseract`) by `SerialPrograms/CMakeLists.txt` — no manual OpenCV build is required on Linux (unlike Windows/Mac, which use prebuilt binaries from the `Packages` repo).

## 2. Build Qt 6.8.3 from source

```bash
mkdir -p ~/src && cd ~/src
wget https://download.qt.io/official_releases/qt/6.8/6.8.3/single/qt-everywhere-src-6.8.3.tar.xz
tar xf qt-everywhere-src-6.8.3.tar.xz
cd qt-everywhere-src-6.8.3

mkdir build && cd build
../configure -prefix "$HOME/qt/6.8.3" -release -opensource -confirm-license \
    -nomake examples -nomake tests \
    -skip qtwebengine -skip qt3d -skip qtwayland

cmake --build . --parallel "$(nproc)"
cmake --install .
```

This builds the full "everywhere" module set (qtbase, qtdeclarative, qtmultimedia, qtserialport, qtsvg, qttools, qtwebsockets, etc.), which is what this project needs (`Widgets`, `SerialPort`, `Multimedia`, `MultimediaWidgets`, `OpenGLWidgets`, `Qml`, `Quick`, `QuickWidgets`). Expect this step to take a long time (well over an hour on most ARM64 boards) and to need ~20-30 GB of free disk space during the build.

If you already have a Qt 6.8.x install (e.g. built previously, or via `aqtinstall`/a distro package new enough to include the above modules), you can skip this step — you just need its install prefix for step 4.

## 3. Get the `Resources` folder (Packages repo)

`SerialPrograms/CMakeLists.txt` expects a sibling `Packages` checkout next to this repo (i.e. `Arduino-Source/Packages`, not a `Resources` folder copied inside `SerialPrograms/`):

```bash
cd /path/to/Arduino-Source   # the repo root, containing SerialPrograms/, Common/, etc.
git clone https://github.com/PokemonAutomation/Packages
```

CMake copies `Packages/Resources` into the build output automatically on first configure/build.

## 4. Configure and build

From the repo root:

```bash
cmake -S SerialPrograms -B build/RelWithDebInfo -G Ninja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_PREFIX_PATH="$HOME/qt/6.8.3"

cmake --build build/RelWithDebInfo --parallel "$(nproc)"
```

Notes:
- **Discord Social SDK is auto-disabled on Linux ARM64.** Discord doesn't offer a Linux ARM64 build of the Social SDK (per their platform-compatibility docs, Linux support is x86_64-only; Windows ARM64 and macOS ARM64 are both fully supported). `CMakeLists.txt` defaults the `PA_SOCIAL_SDK` option to `OFF` automatically when it detects a non-Apple ARM/aarch64 target, so you don't need to pass anything for this — it's mentioned here only so you know why `Compiling with Discord social SDK integration disabled` shows up during configure. You can still force it on/off explicitly with `-DPA_SOCIAL_SDK=ON` or `=OFF` on any platform.
- **ONNX Runtime** is downloaded automatically for you (aarch64 build, from the official `microsoft/onnxruntime` GitHub releases) if you don't already have one. If you want to point at a pre-downloaded copy instead, pass `-DONNX_ROOT_PATH=/path/to/onnxruntime-linux-aarch64-1.23.0`.
- **sdbus-c++**: the project links a bundled static library rather than the system `libsdbus-c++-dev` package. Since a static archive is native object code, it's arch-specific — `CMakeLists.txt` now picks `3rdParty/sdbus-cpp/lib/libsdbus-c++-aarch64.a` or `...-x86_64.a` based on `CMAKE_SYSTEM_PROCESSOR` (previously there was a single `libsdbus-c++.a` that only shipped x86_64 objects, which failed to link on ARM64 with `wrong ELF class` errors). It links against your system's `libsystemd`, so `libdbus-1-dev` above is enough; a separate `libsdbus-c++-dev` install isn't used by the build even though it's harmless to have.
- `DPP` (D++ / Discord bot library) is optional and auto-detected via `pkg-config`; it isn't installed by the apt commands above, so you'll see `Compiling with DPP integration disabled`, which is expected and fine.

The two binaries are produced at `build/RelWithDebInfo/SerialPrograms` (GUI) and `build/RelWithDebInfo/SerialProgramsCommandLine`.

## 5. Run

```bash
./build/RelWithDebInfo/SerialPrograms
```

Only the command-line tool and a headless (`QT_QPA_PLATFORM=offscreen`) launch of the GUI binary were exercised while verifying this build — both start up and resolve all shared libraries correctly (checked with `ldd`). Actual on-screen GUI behavior (e.g. the video-preview flicker noted for x86_64 Ubuntu in `Build-Ubuntu-Qt6.8.2.md`) has not been separately verified on ARM64.

## Known ARM64-specific code changes

For context, the following portability fixes (already applied in this checkout) were needed to get this platform building; they matter if you're diffing against an older `main`:

- `Common/Cpp/CpuId` and `Common/Cpp/Hardware`: the `arm64`/`Hardware_arm64_Linux.tpp` code previously only handled Apple Silicon (`sysctlbyname`). It now branches on `__APPLE__` vs. Linux, reading `/proc/cpuinfo` and `/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq` on Linux.
- Several NEON kernel files (`SerialPrograms/Source/Kernels/**/*_arm64_NEON.*`) had signed/unsigned NEON intrinsic type mismatches that GCC's aarch64 target is stricter about than Apple Clang; these were fixed with the correct `vreinterpretq_*` casts.
- `SerialPrograms/CMakeLists.txt`: `CMAKE_SYSTEM_PROCESSOR` matching was widened from `arm` to `arm|arm64|aarch64` (Linux reports `aarch64`, not `arm64`); a `PA_SOCIAL_SDK` option was added to make the Discord SDK optional, defaulting to `OFF` automatically on non-Apple ARM/aarch64 targets since Discord has no Linux ARM64 build (still overridable with `-DPA_SOCIAL_SDK=ON/OFF`); and both the Linux ONNX Runtime auto-download and the bundled sdbus-c++ static library are now selected per-architecture (`aarch64` vs `x86_64`) instead of assuming x86_64.
- `3rdParty/sdbus-cpp/lib/`: split into `libsdbus-c++-x86_64.a` and `libsdbus-c++-aarch64.a` (previously a single `libsdbus-c++.a` that only contained x86_64 object code — prebuilt static archives are native code, so one file can't serve both architectures). `CMakeLists.txt` picks the right one automatically.
- `OCR_RawTesseractOCR.cpp`: fixed a signed-`char` bug in the ASCII-path check (`char` is unsigned by default on aarch64 GCC, so `ch < 0` never matched non-ASCII bytes there).

<hr>

**Discord Server:**

[<img src="https://canary.discordapp.com/api/guilds/695809740428673034/widget.png?style=banner2">](https://discord.gg/cQ4gWxN)
