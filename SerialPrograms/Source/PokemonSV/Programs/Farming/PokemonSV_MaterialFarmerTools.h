/*  Material Farmer - Happiny dust
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_MaterialFarmerTools_H
#define PokemonAutomation_PokemonSV_MaterialFarmerTools_H

#include <functional>
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "PokemonSV/Options/PokemonSV_EncounterBotCommon.h"
#include "PokemonSV/Options/PokemonSV_SandwichMakerOption.h"
#include "PokemonSV/Programs/ShinyHunting/PokemonSV_LetsGoTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

struct MaterialFarmerOptions {
    BooleanCheckBoxOption& save_game_before_sandwich_option;
    SimpleIntegerOption<uint16_t>& num_sandwich_rounds_option;
    OCR::LanguageOCROption& language_option;
    SandwichMakerOption& sandwich_options;
    GoHomeWhenDoneOption& go_home_when_done_option;
    FloatingPointOption& auto_heal_percent_option;
    BooleanCheckBoxOption& save_debug_video_option;
    BooleanCheckBoxOption& skip_warp_to_pokecenter_option;
    BooleanCheckBoxOption& skip_sandwich_option;
    EventNotificationOption& notification_status_update_option;
    EventNotificationOption& notification_error_recoverable_option;
    EventNotificationsOption& notifications_option;
};

void run_material_farmer(SingleSwitchProgramEnvironment& env, BotBaseContext& context, MaterialFarmerOptions options);

void run_one_sandwich_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context, 
    LetsGoEncounterBotTracker& encounter_tracker, MaterialFarmerOptions& options);

void move_to_start_position_for_letsgo0(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

void move_to_start_position_for_letsgo1(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

void lets_go_movement0(BotBaseContext& context);

void lets_go_movement1(BotBaseContext& context);

bool is_sandwich_expired(WallClock last_sandwich_time);

void run_lets_go_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context, LetsGoEncounterBotTracker& encounter_tracker);

void run_from_battles_and_back_to_pokecenter(SingleSwitchProgramEnvironment& env, BotBaseContext& context, 
    std::function<void(SingleSwitchProgramEnvironment& env, BotBaseContext& context)>&& action);

}
}
}
#endif
