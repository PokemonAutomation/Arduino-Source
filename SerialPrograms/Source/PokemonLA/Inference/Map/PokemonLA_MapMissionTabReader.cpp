/*  Map Zoom Level Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonLA_MapMissionTabReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

bool is_map_mission_tab_raised(const ImageViewRGB32& screen){
    // The white area around the "R" button when the tab is raise.
    const ImageFloatBox box0{0.9235, 0.617, 0.003, 0.019};
    const ImageFloatBox box1{0.937, 0.62, 0.0035, 0.012};

    return is_white(image_stats(extract_box_reference(screen, box0))) || 
        is_white(image_stats(extract_box_reference(screen, box1)));
}

}
}
}
