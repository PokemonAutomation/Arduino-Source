/*  BDSP Blink Scenes
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string>
#include "Common/Cpp/Exceptions.h"
#include "PokemonBDSP/Options/PokemonBDSP_PlayerModelOption.h"
#include "PokemonBDSP_BlinkScenes.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::vector<BdspEyeTemplate> lake_eye_templates(uint8_t player_model){
    if (player_model < 1 || player_model > BDSP_PLAYER_MODEL_COUNT){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Unknown player model: " + std::to_string(player_model)
        );
    }
    return {
        BdspEyeTemplate{
            "lake_templates/model" + std::to_string(player_model) + ".png",
            {0.5026, 0.4472, 0.0240, 0.0426},
            "Player"
        },
        BdspEyeTemplate{
            "lake_templates/barry.png",
            {0.5521, 0.4528, 0.0214, 0.0380},
            "Barry"
        },
    };
}


std::vector<BdspEyeTemplate> bedroom_eye_templates(uint8_t player_model){
    if (player_model < 1 || player_model > BDSP_PLAYER_MODEL_COUNT){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Unknown player model: " + std::to_string(player_model)
        );
    }
    return {
        BdspEyeTemplate{
            "bedroom_templates/model" + std::to_string(player_model) + ".png",
            {0.4708, 0.4500, 0.0214, 0.0491},
            "Player"
        },
    };
}


}
}
}
