/*  Status Info Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Detect shiny and alpha on the Status Info Screen.
 */

#ifndef PokemonAutomation_PokemonLA_StatusInfoScreenDetector_H
#define PokemonAutomation_PokemonLA_StatusInfoScreenDetector_H

#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ButtonDetector.h"
#include "PokemonLA/Resources/PokemonLA_PokemonInfo.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


PokemonDetails read_status_info(
    LoggerQt& logger, VideoOverlay& overlay,
    const ImageViewRGB32& frame,
    Language language
);




}
}
}
#endif
