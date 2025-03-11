/*  Map Zoom Level Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#ifndef PokemonAutomation_PokemonLA_MapZoomLevelReader_H
#define PokemonAutomation_PokemonLA_MapZoomLevelReader_H

namespace PokemonAutomation{
    class ImageViewRGB32;
namespace NintendoSwitch{
namespace PokemonLA{


// Return the zoom level of the map by checking the location of the yellow disk in
// the zoom gauge.
// zoom level 0: full Hisui map view
// zoom level 1: full region map view
// zoom level 2: local view
// Return -1 if the reading fails.
int read_map_zoom_level(const ImageViewRGB32& screen);


}
}
}
#endif
