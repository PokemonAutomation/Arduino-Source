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
#include "PokemonSV/Inference/Boxes/PokemonSV_IvJudgeReader.h"
#include "PokemonSV/Options/PokemonSV_EncounterBotCommon.h"
#include "PokemonSV/Options/PokemonSV_SandwichMakerOption.h"
#include "PokemonSV/Programs/ShinyHunting/PokemonSV_LetsGoTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class MaterialFarmerOptions : public GroupOption{
public:
    MaterialFarmerOptions()
        : GroupOption(
            "Material Farmer",
            LockMode::UNLOCK_WHILE_RUNNING,
            false, true
        )
        , SAVE_GAME_BEFORE_SANDWICH(
            "<b>Save Game before each sandwich:</b><br>"
            "Recommended to leave on, as the sandwich maker will reset the game if it detects an error.",
            LockMode::LOCK_WHILE_RUNNING,
            true
        )
        , NUM_SANDWICH_ROUNDS(
            "<b>Number of sandwich rounds to run:</b><br>"
            "400-650 Happiny dust per sandwich, with Normal Encounter power level 2.<br>"
            "(e.g. Chorizo x4, Banana x2, Mayo x3, Whipped Cream x1)",
            LockMode::UNLOCK_WHILE_RUNNING,
            3
        )
        , LANGUAGE(
            "<b>Game Language:</b><br>Required to read sandwich ingredients.",
            IV_READER().languages(),
            LockMode::UNLOCK_WHILE_RUNNING,
            false
        )
        , SANDWICH_OPTIONS(LANGUAGE)
        , GO_HOME_WHEN_DONE(true)
        , AUTO_HEAL_PERCENT(
            "<b>Auto-Heal %</b><br>Auto-heal if your HP drops below this percentage.",
            LockMode::UNLOCK_WHILE_RUNNING,
            75, 0, 100
        )
        , SAVE_DEBUG_VIDEO(
            "<b>DEV MODE: Save debug videos to Switch:</b><br>"
            "Set this on to save a Switch video everytime an error occurs. You can send the video to developers to help them debug later.",
            LockMode::LOCK_WHILE_RUNNING,
            false
        )
        , SKIP_WARP_TO_POKECENTER(
            "<b>DEV MODE: Skip warping to closest PokeCenter:</b><br>"
            "This is for debugging the program without waiting for the initial warp.",
            LockMode::LOCK_WHILE_RUNNING,
            false
        )
        , SKIP_SANDWICH(
            "<b>DEV MODE: Skip making sandwich:</b><br>"
            "This is for debugging the program without waiting for sandwich making.",
            LockMode::UNLOCK_WHILE_RUNNING,
            false
        )
        , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
        , NOTIFICATION_PROGRAM_FINISH(
            "Program Finished",
            true, true,
            ImageAttachmentMode::JPG,
            {"Notifs"}
        )
        , NOTIFICATION_ERROR_RECOVERABLE(
            "Program Error (Recoverable)",
            true, false,
            ImageAttachmentMode::PNG,
            {"Notifs"}

        )
        , NOTIFICATION_ERROR_FATAL(
            "Program Error (Fatal)",
            true, true,
            ImageAttachmentMode::PNG,
            {"Notifs"}
        )
        , NOTIFICATIONS({
            &NOTIFICATION_STATUS_UPDATE,
            &NOTIFICATION_PROGRAM_FINISH,
            &NOTIFICATION_ERROR_RECOVERABLE,
            &NOTIFICATION_ERROR_FATAL,
        })
    {
        if (PreloadSettings::instance().DEVELOPER_MODE){
            PA_ADD_OPTION(SAVE_DEBUG_VIDEO);
            PA_ADD_OPTION(SKIP_WARP_TO_POKECENTER);
            PA_ADD_OPTION(SKIP_SANDWICH);
        }
        PA_ADD_OPTION(SAVE_GAME_BEFORE_SANDWICH);
        PA_ADD_OPTION(NUM_SANDWICH_ROUNDS);
        PA_ADD_OPTION(LANGUAGE);
        PA_ADD_OPTION(SANDWICH_OPTIONS);
        PA_ADD_OPTION(GO_HOME_WHEN_DONE);
        PA_ADD_OPTION(AUTO_HEAL_PERCENT);
        PA_ADD_OPTION(NOTIFICATIONS);
    }

    BooleanCheckBoxOption SAVE_GAME_BEFORE_SANDWICH;

    SimpleIntegerOption<uint16_t> NUM_SANDWICH_ROUNDS;
    
    OCR::LanguageOCROption LANGUAGE;

    SandwichMakerOption SANDWICH_OPTIONS;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    FloatingPointOption AUTO_HEAL_PERCENT;

    // Debug options
    BooleanCheckBoxOption SAVE_DEBUG_VIDEO;
    BooleanCheckBoxOption SKIP_WARP_TO_POKECENTER;
    BooleanCheckBoxOption SKIP_SANDWICH;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationOption NOTIFICATION_ERROR_RECOVERABLE;
    EventNotificationOption NOTIFICATION_ERROR_FATAL;
    EventNotificationsOption NOTIFICATIONS;
};

void run_material_farmer(SingleSwitchProgramEnvironment& env, BotBaseContext& context, MaterialFarmerOptions& options);

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
