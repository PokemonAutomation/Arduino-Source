/*  Status Info Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Detect shiny and alpha on the Status Info Screen in a battle.
 */

#ifndef PokemonAutomation_PokemonLA_StatusInfoScreenDetector_H
#define PokemonAutomation_PokemonLA_StatusInfoScreenDetector_H

#include "CommonFramework/Language.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonLA/Resources/PokemonLA_PokemonInfo.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


PokemonDetails read_status_info(
    Logger& logger, VideoOverlay& overlay,
    const ImageViewRGB32& frame,
    Language language
);




}
}
}
#endif
