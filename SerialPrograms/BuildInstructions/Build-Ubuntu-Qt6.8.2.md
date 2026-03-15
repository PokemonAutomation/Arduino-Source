# How to Build (Qt 6.8.2) - Linux

Note that our Linux setup might not work. The video display can flicker to the point of being unusable.

## Build Tools:

Install the following build requirements, these steps will use ubuntu packages but the steps will be the same on your distro:

```
git cmake ninja-build gcc g++ qt6-base-dev qt6-multimedia-dev qt6-serialport-dev libopencv-dev libonnx-dev libasound2-dev
dpkg                    # To build .deb packages for Debian based distros
rpm                     # To build .rpm packages for RHEL based distros
pacman-package-manager  # To build .pkg packages for Arch based distros
```


## Install Qt 6.8.2:

1. Download the online installer from here: https://www.qt.io/download-qt-installer
2. Select custom install settings.
3. Select the following options: 
    - Qt 6.8.2
        - MSVC 2022 64-bit
        - Sources
        - Additional Libraries
            - Qt Image Formats
            - Qt Multimedia
            - Qt Serial Port
        - Qt Debug Information Files   

![](Images/Windows-Install-Qt6.7.3-Custom.png)
![](Images/Windows-Install-Qt6.8.2-Components.png)

### Alternatively:
1. ```curl -L https://download.qt.io/official_releases/online_installers/qt-online-installer-linux-x64-online.run -o qt.run```
2. ```chmod +x qt.run```
3. ```./qt.run install qt6.8.2-full-dev```
4. ```export CMAKE_PREFIX_PATH=/opt/Qt/6.8.2/gcc_64:$CMAKE_PREFIX_PATH```

## Setup:

### Package:
1. Clone this repo.
2. Open a terminal in the folder ```Arduino-Source/SerialPrograms/Scripts/Linux```
3. Run the script ```./packages.sh (PACKAGE TYPE)```
   - PACKAGE TYPE:
     - DEB - .deb package
     - RPM - .rpm package
     - PKG - .pkg package
     - TGZ - Generic package
4. Find the package at ```/Arduino-Source/SerialPrograms/build```
5. Install the package ```apt install ./pokemon-automation--x86_64.deb```

### Source:
1. Clone this repo.
2. Open a terminal in ```Arduino-Source/SerialPrograms```
3. Compile the source code ```cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-O2 -march=native -Wno-odr" -DCMAKE_CXX_FLAGS="-O2 -march=native -Wno-odr" && ninja -C build```

<hr>

**Discord Server:** 


[<img src="https://canary.discordapp.com/api/guilds/695809740428673034/widget.png?style=banner2">](https://discord.gg/cQ4gWxN)

