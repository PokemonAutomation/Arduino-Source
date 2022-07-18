/*  Box Gender Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BoxGenderDetector_H
#define PokemonAutomation_PokemonBDSP_BoxGenderDetector_H

#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "Pokemon/Inference/Pokemon_IVCheckerReader.h"
#include "PokemonBDSP/Options/PokemonBDSP_EggHatchFilter.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


EggHatchGenderFilter read_gender_from_box(
    LoggerQt& logger, VideoOverlay& overlay,
    const ImageViewRGB32& frame
);




}
}
}
#endif
