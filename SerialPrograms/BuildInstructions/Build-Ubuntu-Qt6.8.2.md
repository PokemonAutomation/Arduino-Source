# How to Build (Qt 6.8.2) - Ubuntu 24.04

Note that our Ubuntu setup does not work. The video display flickers to the point of being unusable.

## Build Tools:

Install the following packages:

```
sudo apt install cmake
sudo apt install libglx-dev libgl1-mesa-dev
sudo apt install libopencv-dev
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

## Setup:

1. Clone this repo.
2. Clone the [Packages Repo](https://github.com/PokemonAutomation/Packages).
3. In the `Packages` repo, copy the `SerialPrograms/Resources` folder into the root of the `Arduino-Source` repo.

![](Images/Directory.png)

4. Open Qt Creator.
5. Click on `File` -> `Open File or Project`.
6. Navigate to `SerialPrograms` and select `CMakeLists.txt`.
7. Enable parallel build: Build & Run -> Build Steps -> Build -> Details -> CMake arguments: `-j16` (the # of cores you have)
8. At the bottom left corner, click on the little monitor and select `Release with Debug Information`.
9. Still in the bottom left corner, click the upper green arrow to compile and launch the program.

<hr>

**Discord Server:** 


[<img src="https://canary.discordapp.com/api/guilds/695809740428673034/widget.png?style=banner2">](https://discord.gg/cQ4gWxN)

