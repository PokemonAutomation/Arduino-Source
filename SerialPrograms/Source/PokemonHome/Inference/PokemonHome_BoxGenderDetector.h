/*  Box Gender Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_BoxGenderDetector_H
#define PokemonAutomation_PokemonHome_BoxGenderDetector_H

#include "Pokemon/Options/Pokemon_StatsHuntFilter.h"

namespace PokemonAutomation{

class ImageViewRGB32;
class VideoOverlaySet;

namespace NintendoSwitch{
namespace PokemonHome{

// Detect gender symbol inside the pokemon storage box
class BoxGenderDetector{
public:
    static void make_overlays(VideoOverlaySet& items);
    
    static Pokemon::StatsHuntGenderFilter detect(const ImageViewRGB32& screen);
};



}
}
}

#endif
