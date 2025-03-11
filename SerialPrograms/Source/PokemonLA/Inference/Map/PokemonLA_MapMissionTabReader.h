/*  Map Mission Tab Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#ifndef PokemonAutomation_PokemonLA_MapMissionTabReader_H
#define PokemonAutomation_PokemonLA_MapMissionTabReader_H

namespace PokemonAutomation{
    class ImageViewRGB32;
namespace NintendoSwitch{
namespace PokemonLA{


// Whether the Missions & Requests tab is raised on map view.
// The detection is most reliable when map is in zoom level 1: the full region map view.
// Suggest using this detection only when zoom level is 1.
bool is_map_mission_tab_raised(const ImageViewRGB32& screen);


}
}
}
#endif
