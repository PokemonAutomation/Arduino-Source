/*  Material Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <atomic>
#include <sstream>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_LetsGoHpReader.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IvJudgeReader.h"
#include "PokemonSV/Inference/Battles/PokemonSV_EncounterWatcher.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/Battles/PokemonSV_Battles.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_SandwichRoutines.h"
#include "PokemonSV/Programs/ShinyHunting/PokemonSV_LetsGoTools.h"
#include "PokemonSV_MaterialFarmer.h"
#include "PokemonSV_MaterialFarmerTools.h"

// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



MaterialFarmer_Descriptor::MaterialFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:MaterialFarmer",
        STRING_POKEMON + " SV", "Material Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/MaterialFarmer.md",
        "Farm materials - Happiny dust from Chanseys/Blisseys, for Item Printer.",
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
std::unique_ptr<StatsTracker> MaterialFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



MaterialFarmer::MaterialFarmer()
    : SAVE_GAME_BEFORE_SANDWICH(
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


void MaterialFarmer::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    


    assert_16_9_720p_min(env.logger(), env.console);

    //  Connect the controller.
    pbf_press_button(context, BUTTON_L, 10, 50);

    // Throw user setup errors early in program
    // - Ensure language is set
    const Language language = LANGUAGE;
    if (language == Language::None) {
        throw UserSetupError(env.console.logger(), "Must set game language option to read ingredient lists.");
    }

    // - Ensure audio input is enabled
    LetsGoKillSoundDetector audio_detector(env.console, [](float){ return true; });
    wait_until(
        env.console, context,
        std::chrono::milliseconds(1100),
        {
            static_cast<AudioInferenceCallback&>(audio_detector)
        }
    );
    audio_detector.throw_if_no_sound(std::chrono::milliseconds(1000));

    // start by warping to pokecenter for positioning reasons
    if (!SKIP_WARP_TO_POKECENTER){
        reset_to_pokecenter(env.program_info(), env.console, context);
    }

    MaterialFarmerOptions options {
        SAVE_GAME_BEFORE_SANDWICH,
        NUM_SANDWICH_ROUNDS,
        LANGUAGE,
        SANDWICH_OPTIONS,
        GO_HOME_WHEN_DONE, 
        AUTO_HEAL_PERCENT,
        SAVE_DEBUG_VIDEO, 
        SKIP_WARP_TO_POKECENTER,
        SKIP_SANDWICH,
        NOTIFICATION_STATUS_UPDATE,
        NOTIFICATION_ERROR_RECOVERABLE,
        NOTIFICATIONS
    };

    run_material_farmer(env, context, options);
    
}




}
}
}
