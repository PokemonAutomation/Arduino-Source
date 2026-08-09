/*  Player Model Select
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string>
#include <vector>
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "PokemonBDSP_PlayerModelOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


namespace{

struct PlayerModelIcons{
    std::vector<ImageRGB32> icons;
    StringSelectDatabase database;

    PlayerModelIcons(){
        icons.reserve(BDSP_PLAYER_MODEL_COUNT);
        for (uint8_t model = 1; model <= BDSP_PLAYER_MODEL_COUNT; model++){
            std::string number = std::to_string(model);
            icons.emplace_back(
                RESOURCE_PATH() + "PokemonBDSP/Rng/model_icons/model" + number + ".png"
            );
            database.add_entry(StringSelectEntry(
                "model-" + number, "Model " + number, icons.back()
            ));
        }
    }
};
const PlayerModelIcons& PLAYER_MODEL_ICONS(){
    static PlayerModelIcons icons;
    return icons;
}
}


PlayerModelOption::PlayerModelOption()
    : StringSelectOption(
        "<b>Character model:</b><br>The appearance chosen for the player character. "
        "Each one is watched through a different eye template.",
        PLAYER_MODEL_ICONS().database,
        LockMode::LOCK_WHILE_RUNNING,
        "model-1"
    )
{}


}
}
}
