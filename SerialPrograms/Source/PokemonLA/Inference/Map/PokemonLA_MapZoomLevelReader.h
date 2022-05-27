/*  Selected Region Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#ifndef PokemonAutomation_PokemonLA_SelectedRegionDetector_H
#define PokemonAutomation_PokemonLA_SelectedRegionDetector_H

class QImage;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


// Return the zoom level of the map by checking the location of the yellow disk in
// the zoom gauge.
// zoom level 0: full Hisui map view
// zoom level 1: full region map view
// zoom level 2: local view
// Return -1 if the reading fails.
int read_map_zoom_level(const QImage& screen);


}
}
}
#endif
