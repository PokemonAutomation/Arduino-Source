# How to Build (Qt 6.8.3) - Windows

**(Note: These instructions have not been fully tested for 6.8.3. Let us know if you encounter any issues.)**

## Build Tools:

The installation order here is important. While other orderings may work, this is the specific order that we have tested. And the Qt installation must be the last thing installed.

1. Install Visual Studio 2022:
    1. [Download Page](https://docs.microsoft.com/en-us/visualstudio/releases/2022/release-notes)
    2. Make sure you select the C++ development tools.
2. Install Windows Development SDK:
    1. [Download Page](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/)
3. Install CMake:
    1. [Download Page](https://cmake.org/download/)
    2. When prompted select, "Add CMake to the system PATH for all users".

## Install Qt 6.8.3:

Unlike with Qt 5.12, there is no offline installer for it. So you have two options here, use the online installer, or use a copy of an installation.

### Official Online Installer:

If you are ok with creating an account with Qt and using their online installer, then use this method.

1. Download the online installer from here: https://www.qt.io/download-qt-Images/
3. Select the following options: 
    - Qt 6.8.3
        - MSVC 2022 64-bit
        - Sources
        - Additional Libraries
            - Qt Image Formats
            - Qt Multimedia
            - Qt Serial Port
        - Qt Debug Information Files   

![](Images/Windows-Install-Qt6.8.3-Custom.png)
![](Images/Windows-Install-Qt6.8.3-Components.png)

If you repeatedly run into an error involving "SSL handshake failed", you will not be able to use the online installer. Please try the other option.

### Unofficial Installation Copy:

If you are unable or unwilling to use the online installer, the alternative is to copy an installation directly into your system. To do this, you will need to download the installation from us, and copy it into your C drive.

1. Join our [Discord server](https://discord.gg/cQ4gWxN) and ask for the link to the Qt6 standalone. Someone will DM you with a link*.
2. Download `Qt6.8.3.7z` and decompress it. You can use [7-zip](https://www.7-zip.org/) to decompress it. This will create a folder with the same name.
3. Move this folder to `C:\`. It will probably ask you for permissions to do it.
4. Navigate to: `C:\Qt6.8.3\Tools\QtCreator\bin\` and create a shortcut to `qtcreator.exe`. Copy this shortcut to somewhere convenient. (By default this shortcut is named, `Qt Creator 16.0.0 (Community)`)

*This Qt6 standalone file is 3GB in size and is being hosted by our staff for our own developers. We don't want the entire world converging here and overrunning the server.

## Setup:

1. Clone this repo.
2. Clone the [Packages Repo](https://github.com/PokemonAutomation/Packages).
3. In the `Packages` repo, copy the `SerialPrograms/Resources` folder into the root of the `Arduino-Source` repo.

![](Images/Directory.png)

4. Open Qt Creator.
5. Click on `File` -> `Open File or Project`.
6. Navigate to `SerialPrograms` and select `CMakeLists.txt`.
7. It will then ask you to configure the project. Ensure `Desktop Qt 6.8.3 MSVC2022 64bit` and `Release with Debug Information` are selected.
    - Ensure the build directory for `Desktop_Qt_6_8_2_MSVC2022_64bit-RelWithDebInfo` is located in the root of the `Arduino-Source` repo, and not buried in subfolders (e.g. `build` or `SerialPrograms`).
    - Click `Configure Project`.

![](Images/Windows-configure-project-qt-creator-13.png)

8. At the bottom left corner, click on the little monitor and select `Release with Debug Information`.

![](Images/Windows-Configuration-Qt6.png)

9. Still in the bottom left corner, click the upper green arrow to compile and launch the program.

## Troubleshooting

### Failed to open '../3rdPartyBinaries/opencv_world460d.zip', or file missing

- Click `Projects` on the left sidebar.
- In `Build directory`, ensure the build directory for `Desktop_Qt_6_8_3_MSVC2022_64bit-RelWithDebInfo` is located in the root of the `Arduino-Source` repo, and not buried in subfolders (e.g. `build` or `SerialPrograms`).
  - e.g. change the default build directory from:  `Arduino-Source/SerialPrograms/build/Desktop_Qt_6_8_3_MSVC2022_64bit-RelWithDebInfo`
    - to: `Arduino-Source/build-SerialPrograms-Desktop_Qt_6_8_3_MSVC2022_64bit-RelWithDebInfo`


## Upgrading Qt components

### Installing newer Qt version
If you have already have older versions of Qt installed, but need to upgrade Qt components, you can use the Maintenance tool, which comes pre-instaled with Qt (C:\Qt\MaintenanceTool.exe).

- When prompted, in `Maintenance Actions`, choose `Add or remove components`.
- You may select the components needed, as per the instructions above, for the newer version. And de-select old versions to remove them, if you wish.
- Optional: In `Maintenance Actions`, you may also choose `Update components` as well. To update Qt Creator, and other components.

### Building with the newer Qt version
- Open the `SerialPrograms` project in Qt, as outlined in the "Setup" section above.
- In the toolbar, click `Edit` -> `Preferences`
- Under `Kits` tab, choose the newer Qt version (e.g. Desktop Qt 6.8.3)
<img src="Images/QT-kits.png">

- Under `Compiler`, set it to Microsoft Visual C++ Compiler or Visual Studio, if not already set.
- Click `Ok`, to save Kits preferences.
- Click the Monitor symbol at the bottom left
<img src="Images/QT-kit-build.png">

- Under `Kit`: choose the desired Qt version.
- Under `Build`: choose `Release with Debug Information`
- Click the green arrow to compile and launch the program.
- Optional: Move the `UserSettings` folder from the old build folder to the new build folder, to keep your old settings, such as the dev key.

### Upgrade compiler
- Upgrade the compiler by installing a new version of Visual Studio. Ensure `Desktop development with C++` is checked, so the latest MSVC compiler is also installed.
- Open the `SerialPrograms` project in Qt, as outlined in the "Setup" section above.
- In the toolbar, click `Edit` -> `Preferences`
- Click the `Kits` tab on the left. Then click the `Compilers` tab, click `Re-detect` to refresh the list of compilers. Then click `Apply` in the bottom right. 
- Back in the `Kits` tab, your newly installed compilers should now show up in the list of compilers.

When building, if you get the error `yvals_core.h:931: error: STL1001: Unexpected compiler version, expected MSVC 19.42 or newer` (or something similar), consider deleting the old build folder, and rebuilding from scratch.

<hr>

**Discord Server:** 


[<img src="https://canary.discordapp.com/api/guilds/695809740428673034/widget.png?style=banner2">](https://discord.gg/cQ4gWxN)

