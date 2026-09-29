/*  BDSP Blink Scenes
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BlinkScenes_H
#define PokemonAutomation_PokemonBDSP_BlinkScenes_H

#include <stdint.h>
#include <vector>
#include "PokemonBDSP_EyeBlinkDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


//  Which eyes to watch, and where they sit, in each scene the RNG programs use.

std::vector<BdspEyeTemplate> lake_eye_templates(uint8_t player_model);

std::vector<BdspEyeTemplate> bedroom_eye_templates(uint8_t player_model);


}
}
}
#endif
