/*  Box Gender Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BoxGenderDetector_H
#define PokemonAutomation_PokemonBDSP_BoxGenderDetector_H

#include "Pokemon/Options/Pokemon_EggHatchFilter.h"

namespace PokemonAutomation{
class Logger;
class VideoOverlay;
class ImageViewRGB32;
namespace NintendoSwitch{
namespace PokemonBDSP{


Pokemon::EggHatchGenderFilter read_gender_from_box(
    Logger& logger, VideoOverlay& overlay,
    const ImageViewRGB32& frame
);




}
}
}
#endif
