/*  Selected Region Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#ifndef PokemonAutomation_PokemonLA_SelectedRegionDetector_H
#define PokemonAutomation_PokemonLA_SelectedRegionDetector_H

#include "PokemonLA/PokemonLA_Locations.h"

namespace PokemonAutomation{
    class BotBaseContext;
    class ConsoleHandle;
namespace NintendoSwitch{
namespace PokemonLA{


MapRegion detect_selected_region(ConsoleHandle& console, BotBaseContext& context);



}
}
}
#endif
