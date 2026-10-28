# Include Sort Order

The include order convention is roughly as follows:

1. Absolute path includes go first. An absolute path is relative to either the repo root or one of the include directories that's passed as a compiler flag.
2. Relative paths. These will always be relative to the same folder as the current file. Backwards includes (`../`) are banned outside of code that is shared with firmware.
3. Includes in the same folder (with no `/`) will be sorted last.

For files that are within the same category above, sort them according to this natural order:

- Standard C includes (`stdin.h`)
- Standard C++ includes (`vector`)
- OS-specific includes ('Windows.h')
- Non-Qt 3rd party. (`immintrin.h`, `opencv.hpp`, etc...)
- Qt (`QWidget`)
- `3rdParty-Core`
- `3rdParty`
- `Common/Compiler.h`
- `Common/Cpp/`
- `Common/Qt/`
- `Common/CRC32/`
- `Common/PABotBase2/`
- `Common/*` (everything else)
- `Kernels/`
- `CommonFramework/`
- `CommonTools/`
- `ML/`
- `Tests/`
- `ComputerPrograms/`
- `ControllerInput/`
- `Controllers/`
- `GameConsole/`
- `Integrations/`
- `NintendoSwitch/`
- `Pokemon/`
- `PokemonBDSP/`
- `VideoGames/`
- `VideoGames/PokemonHome/`
- `VideoGames/PokemonLGPE/`
- `VideoGames/PokemonSwSh/`
- `VideoGames/PokemonLA/`
- `VideoGames/PokemonSV/`
- `VideoGames/PokemonLZA/`
- `VideoGames/PokemonFRLG/`
- `VideoGames/PokemonPokopia/`
- `VideoGames/PokemonRSE/`
- `VideoGames/ZeldaTotK/`

This list is not exhaustive.

The pattern is that directories with fewer dependencies are at top (core libraries), while those with more dependencies (apps) are at the bottom. Naturally, a .cpp's own header will almost always be the last include. The order within `VideoGames/` is in approximate order of release date with Pokemon games sorted first.

Include orders within each bullet are not specified, though many of them also have natural suborderings.


**Debugging Exception:**

Files that are actively being debugged may have this after all the includes. If the `<iostream>` not being used for non-debugging code, then leave this pattern intact and do not pull it into the sorted includes above.
```
#include <iostream>
using std::cout;
using std::endl;
```

**List Exception:**

For files that consist of long lists of things that map 1-to-1 with a header (such as a panel list file), the include order of the list should match the order they are used in the file.
