/*  Box Gender Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BoxGenderDetector_H
#define PokemonAutomation_PokemonSwSh_BoxGenderDetector_H

#include "Pokemon/Options/Pokemon_EggHatchFilter.h"

namespace PokemonAutomation{

class ImageViewRGB32;
class VideoOverlaySet;

namespace NintendoSwitch{
namespace PokemonSwSh{

// Detect gender symbol inside the pokemon storage box
class BoxGenderDetector{
public:
    static void make_overlays(VideoOverlaySet& items);

    static Pokemon::EggHatchGenderFilter detect(const ImageViewRGB32& screen);
};



}
}
}

#endif

