/*  Selected Region Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#ifndef PokemonAutomation_PokemonLA_SelectedRegionDetector_H
#define PokemonAutomation_PokemonLA_SelectedRegionDetector_H

namespace PokemonAutomation{
    class ProgramEnvironment;
    class ConsoleHandle;
namespace NintendoSwitch{
namespace PokemonLA{


enum class MapRegion{
    NONE,
    JUBILIFE,
    FIELDLANDS,
    MIRELANDS,
    COASTLANDS,
    HIGHLANDS,
    ICELANDS,
    RETREAT,
};
extern const char* MAP_REGION_NAMES[];

MapRegion detect_selected_region(ProgramEnvironment& env, ConsoleHandle& console);


}
}
}
#endif
