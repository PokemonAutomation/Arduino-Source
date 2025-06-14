/*  Blueberry Quests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_BlueberryCatchPhoto_H
#define PokemonAutomation_PokemonSV_BlueberryCatchPhoto_H

#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSV/Programs/Farming/PokemonSV_BlueberryQuests.h"
#include "PokemonSV/Options/PokemonSV_BBQOption.h"

using namespace std;

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{

//Navigate to a photo target
CameraAngle quest_photo_navi(
    const ProgramInfo& info,
    ConsoleHandle& console, ProControllerContext& context,
    const BBQOption& BBQ_OPTIONS,
    BBQuests current_quest
);

//Take picture of a pokemon/location
void quest_photo(
    const ProgramInfo& info,
    ConsoleHandle& console, ProControllerContext& context,
    const BBQOption& BBQ_OPTIONS,
    BBQuests current_quest
);


//Navigate to a catch target
void quest_catch_navi(
    const ProgramInfo& info,
    ConsoleHandle& console, ProControllerContext& context,
    const BBQOption& BBQ_OPTIONS,
    BBQuests current_quest
);

//Select and throw ball
void quest_catch_throw_ball(
    const ProgramInfo& info,
    ConsoleHandle& console, ProControllerContext& context,
    Language language,
    const std::string& selected_ball
);

//Handle catching the target
void quest_catch_handle_battle(
    const ProgramInfo& info,
    ConsoleHandle& console, ProControllerContext& context,
    const BBQOption& BBQ_OPTIONS,
    BBQuests current_quest
);

//Catch a pokemon
void quest_catch(
    const ProgramInfo& info,
    ConsoleHandle& console, ProControllerContext& context,
    const BBQOption& BBQ_OPTIONS,
    BBQuests current_quest
);


//Handle battles for tera-self/defeat-wild-tera. Use first attack until victory.
void wild_battle_tera(
    const ProgramInfo& info,
    ConsoleHandle& console, ProControllerContext& context,
    bool& tera_self
);

}
}
}
#endif
