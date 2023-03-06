# Hex Generator

Source code for the hex generator UI.

## How to Build

**Windows:**

1. Install Qt Creator.
    1. Download the [Qt 5.12.10 Offline Installer](https://www.qt.io/offline-installers). [Direct download.](http://download.qt.io/official_releases/qt/5.12/5.12.10/qt-opensource-windows-x86-5.12.10.exe)
    2. Disconnect from the internet.
    3. Run the installer.
    4. When prompted to select components, at minimum choose the following: ![](Qt-Install-Settings.png)
2. Setup the project.
    1. Run Qt.
    2. Click on `Projects` -> `Open`.
    3. Navigate to [`GeneratorSource`](../GeneratorSource) and select `HexGenerator.pro`.
    4. It will then ask you to configure the project. Select `Desktop Qt 5.12.10 MinGW 32-bit`.
    5. Navigate to `Projects`.
    6. For both build configurations `Debug` and `Release`, uncheck "Shadow Build."
3. Click the green arrow on the bottom left corner to build and run.

Depending on what you are building (Debug or Release), the binary will be created inside either `GeneratorSource/debug/` or `GeneratorSource/release`. Note that at first, you will not be able to run this binary by just double-clicking since they will be missing .dll dependencies. The binary will only run when all the files in [`Deployment`](../Deployment) are in the same directory or on the search `PATH`.

**Linux:**
 - Not supported yet.
 

**Mac:**

1. Install Qt 5
    1. `brew install qt@5`
2. Build from source
    1. `cd` to this source directory
    2. run `qmake` from `qt@5`: `/opt/homebrew/Cellar/qt@5/5.15.8_2/bin/qmake` to generate `Makefile`
    3. run `make -j 10`
    4. the resulted binary `HexGenerator` would be in current directory
3. Cleanup
    1. run `make clean` to cleanup compiled files
    2. run `make distclean` to cleanup everything including `Makefile` itself
