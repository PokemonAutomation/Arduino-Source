/*  Box Gender Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BoxGenderDetector_H
#define PokemonAutomation_PokemonBDSP_BoxGenderDetector_H

#include "Pokemon/Options/Pokemon_StatsHuntFilter.h"

namespace PokemonAutomation{
class Logger;
class VideoOverlay;
class ImageViewRGB32;
namespace NintendoSwitch{
namespace PokemonBDSP{


Pokemon::StatsHuntGenderFilter read_gender_from_box(
    Logger& logger, VideoOverlay& overlay,
    const ImageViewRGB32& frame
);




}
}
}
#endif
