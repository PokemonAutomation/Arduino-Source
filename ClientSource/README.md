# ClientSource

Source code for client-side (PC-side) of PABotBase.

## How to Build

These instructions will build the Device Logger. To change programs, edit [`Main.cpp`](../ClientSource/Main.cpp#L33).

**Windows:**
1. Install Visual Studio 2019 with C++.
2. Navigate to [`ClientProject/`](../ClientProject).
3. Open `PokemonAutomationClient.sln` with Visual Studio.
4. Press F7 to compile or F5 to build and run.
 
 This will create a folder inside `ClientProject` that will contain the binary.


**Linux:**
1. Install `make` and `g++`.
2. Navigate to [`ClientSource`](../ClientSource).
3. Type `make` and hit enter.
 
 This will create a folder named `Linux` next to `ClientSource` that contains all the binaries and build artifacts.
 

**Mac:**

Not officially supported yet, but the Linux steps might work with little to no modifications.
