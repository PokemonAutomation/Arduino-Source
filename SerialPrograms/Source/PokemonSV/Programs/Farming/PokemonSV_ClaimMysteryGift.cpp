/*  ClaimMysteryGift
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "NintendoSwitch/Inference/NintendoSwitch_SelectedSettingDetector.h"
#include "NintendoSwitch/Inference/NintendoSwitch_ConsoleTypeDetector.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV/Programs/AutoStory/PokemonSV_AutoStory.h"
#include "PokemonSV/Programs/AutoStory/PokemonSV_AutoStoryTools.h"
#include "PokemonSV_ClaimMysteryGift.h"

#include <iostream>
using std::cout;
using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


ClaimMysteryGift_Descriptor::ClaimMysteryGift_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:ClaimMysteryGift",
        STRING_POKEMON + " SV", "Claim Mystery Gift",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/ClaimMysteryGift.md",
        "Claim the Mystery Gift in SV.",
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {SerialPABotBase::OLD_NINTENDO_SWITCH_DEFAULT_REQUIREMENTS}
    )
{}

// std::unique_ptr<StatsTracker> ClaimMysteryGift_Descriptor::make_stats() const{
//     return std::unique_ptr<StatsTracker>(new ClaimMysteryGiftStats());
// }



ClaimMysteryGift::~ClaimMysteryGift(){
    STARTING_POINT.remove_listener(*this);
}

ClaimMysteryGift::ClaimMysteryGift()
    : LANGUAGE(
        "<b>Game Language:</b>",
        PokemonSwSh::IV_READER().languages(),
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , STARTING_POINT(
        "<b>Starting point:",
        {
            {StartingPoint::NEW_GAME,                "new-game",           "New Game: Start after you have selected your username and character appearance"},
            {StartingPoint::IN_MYSTERY_GIFT,         "in-mystery-gift",    "Start in Mystery Gift window, with cursor on the “1” key."},
            {StartingPoint::DONE_TUTORIAL,           "done-tutorial",      "Start in game. But with all menus closed. Tutorial must be completed. You need to be outside, where you can use Pokeportal."},
        },
        LockMode::LOCK_WHILE_RUNNING,
        StartingPoint::NEW_GAME
    )
    , MYSTERY_GIFT_NOTE{
        "Ensure you are logged into a Nintendo account.<br>"
        "You can ignore the options for multiple switches below. This only works for 1 Switch.<br>"
        "Refer to the documentation on github for more details."
    }
    , CODE(
        "<b>Link Code:</b><br>Mystery Gift code. (not case sensitive)<br>"
        "(Box is big so it's easy to land your mouse on.)",
        LockMode::UNLOCK_WHILE_RUNNING,
        "0123", "0123",
        true
    )
    , GO_HOME_WHEN_DONE(true)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(30))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{

    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(STARTING_POINT);
    PA_ADD_OPTION(MYSTERY_GIFT_NOTE);
    PA_ADD_OPTION(CODE);
    PA_ADD_OPTION(SETTINGS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);


    ClaimMysteryGift::on_config_value_changed(this);

    STARTING_POINT.add_listener(*this);
}

void ClaimMysteryGift::on_config_value_changed(void* object){


}

void ClaimMysteryGift::enter_mystery_gift_code(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    env.log("Enter mystery gift code.");

    std::string normalized_code;
    bool force_keyboard_mode = true;
    bool connect_controller_press = false;
    normalize_code(normalized_code, CODE, force_keyboard_mode);

    const FastCodeEntrySettings& settings = SETTINGS;
    enter_code(
        env.console, context,
        settings.keyboard_layout[env.console.index()],
        normalized_code, force_keyboard_mode,
        !settings.skip_plus,
        connect_controller_press
    );    

}

void ClaimMysteryGift::enter_mystery_gift_window(SingleSwitchProgramEnvironment& env, ProControllerContext& context, int menu_index){
    env.console.log("Save game, then try to enter the mystery gift window.", COLOR_YELLOW);
    save_game_from_menu_or_overworld(env.program_info(), env.console, context, false);

    size_t max_attempts = 5;
    for (size_t i = 0; i < max_attempts; i++){
        enter_menu_from_overworld(env.program_info(), env.console, context, menu_index);
        pbf_press_button(context, BUTTON_A, 20, 4 * TICKS_PER_SECOND);
        pbf_press_dpad(context, DPAD_UP, 20, 105);
        pbf_press_button(context, BUTTON_A, 20, 4 * TICKS_PER_SECOND);
        pbf_press_dpad(context, DPAD_DOWN, 20, 105);
        pbf_press_button(context, BUTTON_A, 20, 4 * TICKS_PER_SECOND);
        pbf_press_button(context, BUTTON_A, 20, 10 * TICKS_PER_SECOND);
        try {
            clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10, {CallbackEnum::PROMPT_DIALOG});
        }catch(OperationFailedException&){
            env.console.log("enter_mystery_gift_window: Failed to detect the dialog that leads to the Mystery Gift window. Reset game and re-try.", COLOR_YELLOW);
            reset_game(env.program_info(), env.console, context);
            continue;
        }

        context.wait_for_all_requests();
        context.wait_for(Milliseconds(300));
        // we expect to be within Mystery Gift window, with the keyboard visible and "1" being highlighted
        
        // check whether this is Switch 1 or 2.
        ConsoleType console_type = env.console.state().console_type();
        if (console_type == ConsoleType::Unknown){
            env.console.log("Unknown Switch type. Try to detect.");
            console_type = detect_console_type_from_in_game(env.console, context);
        }

        ImageFloatBox key1_box; 
        ImageFloatBox other_setting1; 
        ImageFloatBox other_setting2; 
        ImageFloatBox background; 

        if (is_switch1(console_type)){
            key1_box = {0.037322, 0.451172, 0.009879, 0.113281};
            other_setting1 = {0.01, 0.451172, 0.009879, 0.113281};
            other_setting2 = {0.02, 0.451172, 0.009879, 0.113281};
            background = {0.0, 0.451172, 0.009879, 0.113281};
        }else if (is_switch2(console_type)){
            key1_box = {0.062706, 0.510763, 0.009901, 0.097847};
            other_setting1 = {0.02, 0.510763, 0.009901, 0.097847};
            other_setting2 = {0.04, 0.510763, 0.009901, 0.097847};   
            background = {0.0, 0.510763, 0.009901, 0.097847};
        }else{
            throw UserSetupError(
                env.console,
                "Please select a valid Switch console type."
            );
        }        
        
        SelectedSettingWatcher key1_selected(key1_box, other_setting1, other_setting2, background);
        int ret = wait_until(
            env.console, context,
            Milliseconds(5000),
            {key1_selected}
        );
        if (ret < 0){  // failed to detect Key 1 being highlighted. Reset game and re-try
            env.console.log("enter_mystery_gift_window: Failed to detect the Mystery Gift window. Reset game and re-try.", COLOR_YELLOW);
            reset_game(env.program_info(), env.console, context);
            continue;
        }       

        
        return;
    }

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "enter_mystery_gift_window(): Failed to reach Mystery Gift screen after several attempts.",
        env.console
    );    
}

void ClaimMysteryGift::run_autostory_until_pokeportal_unlocked(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    AutoStoryOptions options{
        LANGUAGE,
        StarterChoice::FUECOCO,
        NOTIFICATION_STATUS_UPDATE
    };    
    AutoStoryStats stats;  // unused
    for (size_t segment_index = 0; segment_index <= 6; segment_index++){
        ALL_AUTO_STORY_SEGMENT_LIST()[segment_index]->run_segment(env, context, options, stats);
    }
}


void ClaimMysteryGift::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    // ClaimMysteryGiftStats& stats = env.current_stats<ClaimMysteryGiftStats>();


    // Connect controller
    pbf_press_button(context, BUTTON_L, 20, 20);

    // env.console.log("Start Segment " + ALL_AUTO_STORY_SEGMENT_LIST()[get_start_segment_index()]->name(), COLOR_ORANGE);

    if (STARTING_POINT == StartingPoint::NEW_GAME){
        run_autostory_until_pokeportal_unlocked(env, context);
        env.console.log("Done Autostory portion. Pokeportal should now be unlocked.");
        enter_mystery_gift_window(env, context, 2);
        enter_mystery_gift_code(env, context);
    }else if(STARTING_POINT == StartingPoint::IN_MYSTERY_GIFT){
        enter_mystery_gift_code(env, context);
    }else if (STARTING_POINT == StartingPoint::DONE_TUTORIAL){
        enter_mystery_gift_window(env, context, 3);
        enter_mystery_gift_code(env, context);
    }else{
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown STARTING_POINT.");
    }
    
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}






}
}
}
