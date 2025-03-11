/*  Selected Region Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#ifndef PokemonAutomation_PokemonLA_SelectedRegionDetector_H
#define PokemonAutomation_PokemonLA_SelectedRegionDetector_H

#include "CommonFramework/Tools/VideoStream.h"
#include "PokemonLA/PokemonLA_Locations.h"

namespace PokemonAutomation{
    class CancellableScope;
namespace NintendoSwitch{
namespace PokemonLA{

// On the map that you see when you leave village, detect which region the cursor is currently
// floats on.
// The function detects the region by checking which red region name sign is moving up and down.
MapRegion detect_selected_region(VideoStream& stream, CancellableScope& scope);



}
}
}
#endif
