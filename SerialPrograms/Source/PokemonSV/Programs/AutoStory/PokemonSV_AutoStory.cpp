/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <atomic>
#include <sstream>
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/Battles/PokemonSV_Battles.h"
#include "PokemonSV_AutoStory.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;

AutoStory_Descriptor::AutoStory_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:AutoStory",
        STRING_POKEMON + " SV", "Auto Story",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/AutoStory.md",
        "Progress through the mainstory of SV.",
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct AutoStory_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_checkpoint(m_stats["Checkpoint"])
        , m_segment(m_stats["Segment"])
        , m_reset(m_stats["Reset"])
    {
        m_display_order.emplace_back("Checkpoint");
        m_display_order.emplace_back("Segment");
        m_display_order.emplace_back("Reset");
    }
    std::atomic<uint64_t>& m_checkpoint;
    std::atomic<uint64_t>& m_segment;
    std::atomic<uint64_t>& m_reset;
};
std::unique_ptr<StatsTracker> AutoStory_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



AutoStory::~AutoStory(){
    STARTPOINT.remove_listener(*this);
    ENDPOINT.remove_listener(*this);
}

AutoStory::AutoStory()
    : STARTPOINT(
        "<b>Start Point:</b><br>Program will start with this segment.",
        {
            {StartPoint::INTRO_CUTSCENE,        "00_gameintro",         "00: Intro Cutscene"},
            {StartPoint::IN_ROOM,               "01_inroom",            "01: Start in room"},
            {StartPoint::NEMONA_FIRST_BATTLE,   "02_nemonabattle1",     "02: First Nemona Battle"},
            {StartPoint::CATCH_TUTORIAL,        "03_catchtutorial",     "03: Catch Tutorial"},
            {StartPoint::LEGENDARY_RESCUE,      "04_legendaryrescue",   "04: Rescue Legendary"},
            {StartPoint::ARVEN_FIRST_BATTLE,    "05_arvenbattle1",      "05: First Arven Battle"},
            {StartPoint::LOS_PLATOS,            "06_losplatos",         "06: Go to Los Platos"},
            {StartPoint::MESAGOZA_SOUTH,        "07_mesagozasouth",     "07: Go to Mesagoza South"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        StartPoint::INTRO_CUTSCENE
    )
    , ENDPOINT(
        "<b>End Point:</b><br>Program will stop after completing this segment.",
        {
            {EndPoint::IN_ROOM,            "01_pickstarter",       "01: Pick Starter"},
            {EndPoint::NEMONA_FIRST_BATTLE,     "02_nemonabattle1",     "02: First Nemona Battle"},
            {EndPoint::CATCH_TUTORIAL,          "03_catchtutorial",     "03: Catch Tutorial"},
            {EndPoint::LEGENDARY_RESCUE,        "04_legendaryrescue",   "04: Rescue Legendary"},
            {EndPoint::ARVEN_FIRST_BATTLE,      "05_arvenbattle1",      "05: First Arven Battle"},
            {EndPoint::LOS_PLATOS,              "06_losplatos",         "06: Go to Los Platos"},
            {EndPoint::MESAGOZA_SOUTH,          "07_mesagozasouth",     "07: Go to Mesagoza South"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        EndPoint::IN_ROOM
    )
    , STARTERCHOICE(
        "<b>Starter " + STRING_POKEMON + ":",
        {
            {StarterChoice::SPRIGATITO,         "sprigatito",           "Sprigatito (Grass)"},
            {StarterChoice::FUECOCO,            "fuecoco",              "Fuecoco (Fire)"},
            {StarterChoice::QUAXLY,             "quaxly",               "Quaxly (Water)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        StarterChoice::FUECOCO
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
    PA_ADD_OPTION(STARTPOINT);
    PA_ADD_OPTION(ENDPOINT);
    PA_ADD_OPTION(STARTERCHOICE);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);

    AutoStory::value_changed(this);

    STARTPOINT.add_listener(*this);
    ENDPOINT.add_listener(*this);
}

void AutoStory::value_changed(void* object){
    ConfigOptionState state = (STARTPOINT == StartPoint::INTRO_CUTSCENE) || (STARTPOINT == StartPoint::IN_ROOM)
        ? ConfigOptionState::ENABLED
        : ConfigOptionState::HIDDEN;
    STARTERCHOICE.set_visibility(state);
}

void realign_player(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    PlayerRealignMode realign_mode,
    uint8_t move_x, uint8_t move_y, uint8_t move_duration
){
    console.log("Realigning player direction...");

    switch (realign_mode){
    case PlayerRealignMode::REALIGN_NEW_MARKER:
        console.log("Setting new map marker...");
        open_map_from_overworld(info, console, context);
        pbf_press_button(context, BUTTON_ZR, 20, 105);
        pbf_move_left_joystick(context, move_x, move_y, move_duration, 1 * TICKS_PER_SECOND);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_A, 20, 105);
        leave_phone_to_overworld(info, console, context);
        break;
    case PlayerRealignMode::REALIGN_OLD_MARKER:
        open_map_from_overworld(info, console, context);
        leave_phone_to_overworld(info, console, context);
        pbf_press_button(context, BUTTON_L, 20, 105);
        break;
    case PlayerRealignMode::REALIGN_NO_MARKER:
        pbf_move_left_joystick(context, move_x, move_y, move_duration, 1 * TICKS_PER_SECOND);
        pbf_press_button(context, BUTTON_L, 20, 105);
        break;
    }
}

bool run_battle(
    ConsoleHandle& console, 
    BotBaseContext& context,
    BattleStopCondition stop_condition
){
    while (true){
        NormalBattleMenuWatcher battle(COLOR_BLUE);
        SwapMenuWatcher         fainted(COLOR_PURPLE);
        OverworldWatcher        overworld(COLOR_CYAN);
        DialogBoxWatcher        dialog(COLOR_RED, true);
        context.wait_for_all_requests();

        int ret = wait_until(
            console, context,
            std::chrono::seconds(90),
            {battle, fainted, overworld, dialog}
        );
        context.wait_for(std::chrono::milliseconds(100));

        switch (ret){
        case 0: // battle
            console.log("Detected battle menu, spam first move.");
            pbf_mash_button(context, BUTTON_A, 3 * TICKS_PER_SECOND);
            break;
        case 1: // fainted
            // TODO: Handle fainting during battle
            console.log("Detected fainting.");
            return false;
        case 2: // overworld
            console.log("Detected overworld, battle over.");
            if (stop_condition == BattleStopCondition::STOP_OVERWORLD){
                return true;
            }
            break;
        case 3: // dialog
            console.log("Detected dialog.");
            if (stop_condition == BattleStopCondition::STOP_DIALOG){
                return true;
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        default: // timeout
            console.log("run_battle(): Timed out.");
            return false;
        }
    }
}

bool clear_dialog(ConsoleHandle& console, BotBaseContext& context,
    ClearDialogMode mode, uint16_t seconds_timeout
){
    while (true){
        OverworldWatcher    overworld(COLOR_CYAN);
        PromptDialogWatcher prompt(COLOR_YELLOW);
        WhiteButtonWatcher  whitebutton(COLOR_GREEN, WhiteButton::ButtonA, {0.650, 0.650, 0.140, 0.240});
        DialogBoxWatcher    dialog(COLOR_RED, true);
        context.wait_for_all_requests();

        int ret = wait_until(
            console, context,
            std::chrono::seconds(seconds_timeout),
            {overworld, prompt, whitebutton, dialog}
        );
        context.wait_for(std::chrono::milliseconds(100));

        switch (ret){
        case 0: // overworld
            console.log("Detected overworld.");
            if (mode == ClearDialogMode::STOP_OVERWORLD){
                return true;
            }
            break;
        case 1: // prompt
            console.log("Detected prompt.");
            if (mode == ClearDialogMode::STOP_PROMPT){
                return true;
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case 2: // whitebutton
            console.log("Detected white A button.");
            if (mode == ClearDialogMode::STOP_WHITEBUTTON){
                return true;
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case 3: // dialog
            console.log("Detected dialog.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        default:
            console.log("clear_dialog(): Timed out.");
            if (mode == ClearDialogMode::STOP_TIMEOUT){
                return true;
            }
            return false;
        }
    }
}

bool overworld_navigation(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context,
    NavigationStopCondition stop_condition,
    NavigationMovementMode movement_mode,
    uint8_t x, uint8_t y,
    uint16_t seconds_timeout, uint16_t seconds_realign
){
    bool should_realign = true;
    if (seconds_timeout <= seconds_realign){
        seconds_realign = seconds_timeout;
        should_realign = false;
    }

    while (true){
        NormalBattleMenuWatcher battle(COLOR_BLUE);
        DialogBoxWatcher        dialog(COLOR_RED, true);
        context.wait_for_all_requests();

        int ret = run_until(
            console, context,
            [&](BotBaseContext& context){
                for (int i = 0; i < seconds_timeout / seconds_realign; i++){
                    ssf_press_left_joystick(context, x, y, 0, seconds_realign * TICKS_PER_SECOND);
                    if (movement_mode == NavigationMovementMode::DIRECTIONAL_ONLY){
                        pbf_wait(context, seconds_realign * TICKS_PER_SECOND);
                    } else if (movement_mode == NavigationMovementMode::DIRECTIONAL_SPAM_A){
                        for (size_t j = 0; j < seconds_realign; j++){
                            pbf_press_button(context, BUTTON_A, 20, 105);
                        }
                    }
                    if (should_realign){
                        realign_player(info, console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
                    }
                }
            },
            {battle, dialog}
        );
        context.wait_for(std::chrono::milliseconds(100));

        switch (ret){
        case 0: // battle
            console.log("Detected start of battle.");
            run_battle(console, context, BattleStopCondition::STOP_OVERWORLD);
            auto_heal_from_menu_or_overworld(info, console, context, 0, true);
            realign_player(info, console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            break;
        case 1: // dialog
            console.log("Detected dialog.");
            if (stop_condition == NavigationStopCondition::STOP_DIALOG){
                return true;
            }
            break;
        default:
            console.log("overworld_navigation(): Timed out.");
            return false;
        }
    }
}

void mash_button_till_overworld(
    ConsoleHandle& console, 
    BotBaseContext& context, 
    uint16_t button, uint16_t seconds_run
){
    OverworldWatcher overworld(COLOR_CYAN);
    context.wait_for_all_requests();

    int ret = run_until(
        console, context,
        [button, seconds_run](BotBaseContext& context){
            ssf_mash1_button(context, button, seconds_run * TICKS_PER_SECOND);
            pbf_wait(context, seconds_run * TICKS_PER_SECOND);
        },
        {overworld}
    );

    if (ret < 0){
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "mash_button_till_overworld(): Timed out, no recognized state found.",
            true
        );
    }
}

void reset_game(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context, 
    const std::string& error_msg
){
    try{
        pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
        context.wait_for_all_requests();
        reset_game_from_home(info, console, context, 5 * TICKS_PER_SECOND);
    }catch (OperationFailedException& e){
        // To be safe: avoid doing anything outside of game on Switch,
        // make game resetting non error recoverable
        throw FatalProgramException(std::move(e));
    }
}

void config_option(BotBaseContext& context, int change_option_value){
    for (int i = 0; i < change_option_value; i++){
        pbf_press_dpad(context, DPAD_RIGHT, 15, 20);
    }
    pbf_press_dpad(context, DPAD_DOWN,  15, 20);
}

void enter_menu_from_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    int menu_index,
    MenuSide side,
    bool has_minimap
){
    if (!has_minimap){
        pbf_press_button(context, BUTTON_X, 20, 105);
    }

    WallClock start = current_time();
    bool success = false;

    while (true){
        if (current_time() - start > std::chrono::minutes(3)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "enter_menu_from_overworld(): Failed to enter specified menu after 3 minutes.",
                true
            );
        }

        OverworldWatcher overworld(COLOR_CYAN);
        MainMenuWatcher main_menu(COLOR_RED);
        context.wait_for_all_requests();

        int ret = run_until(
            console, context,
            [](BotBaseContext& context){
                for (int i = 0; i < 10; i++){
                    pbf_wait(context, 2 * TICKS_PER_SECOND);
                    pbf_press_button(context, BUTTON_A, 20, 105);
                }
            },
            {overworld, main_menu}
        );
        context.wait_for(std::chrono::milliseconds(100));

        const bool fast_mode = false;
        switch (ret){
        case 0:
            console.log("Detected overworld.");
            pbf_press_button(context, BUTTON_X, 20, 105);
            continue;
        case 1:
            console.log("Detected main menu.");
            success = main_menu.move_cursor(info, console, context, side, menu_index, fast_mode);
            if (success == false){
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT, console,
                    "enter_menu_from_overworld(): Cannot move menu cursor to specified menu.",
                    true
                );
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            return;
        default:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "enter_menu_from_overworld(): No recognized state after 30 seconds.",
                true
            );
        }
    }
}

void AutoStory::checkpoint_save(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
    AutoStory_Descriptor::Stats& stats = env.current_stats<AutoStory_Descriptor::Stats>();
    save_game_from_overworld(env.program_info(), console, context);
    stats.m_checkpoint++;
    env.update_stats();
    send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE, "Saved at checkpoint.");        
}

void AutoStory::segment_01_00(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
    context.wait_for_all_requests();
    console.log("Start Segment 01: Pick Starter", COLOR_ORANGE);
    console.overlay().add_log("Start Segment 01: Pick Starter", COLOR_ORANGE);

    // Stand up from chair and walk to left side of room
    pbf_move_left_joystick(context, 128, 255, 3 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
    pbf_move_left_joystick(context,   0, 128, 6 * TICKS_PER_SECOND, 1 * TICKS_PER_SECOND);

    // set settings
    enter_menu_from_overworld(env.program_info(), console, context, 0, MenuSide::RIGHT, false);

    // TODO: use visual inference to confirm settings
    config_option(context, 1); // Text Speed: Fast
    config_option(context, 1); // Skip Move Learning: On
    config_option(context, 1); // Send to Boxes: Automatic
    config_option(context, 1); // Give Nicknames: Off
    config_option(context, 0); // Vertical Camera Controls: Regular
    config_option(context, 0); // Horiztontal Camera Controls: Regular
    config_option(context, 0); // Camera Support: On
    config_option(context, 0); // Camera Interpolation: Normal
    config_option(context, 0); // Camera Distance: Close
    config_option(context, 1); // Autosave: Off
    config_option(context, 1); // Show Nicknames: Don't show
    config_option(context, 1); // Skip Cutscenes: On
    config_option(context, 0); // Background Music: 10
    config_option(context, 0); // Sound Effects: 10
    config_option(context, 0); // Pokemon Cries: 10
    config_option(context, 0); // Controller Rumble: On
    config_option(context, 1); // Helping Functions: Off
    pbf_mash_button(context, BUTTON_A, 1 * TICKS_PER_SECOND);
    clear_dialog(console, context, ClearDialogMode::STOP_TIMEOUT, 5);
    pbf_mash_button(context, BUTTON_B, 2 * TICKS_PER_SECOND);

    context.wait_for_all_requests();
    console.log("Go downstairs, get stopped by Skwovet");
    console.overlay().add_log("Go downstairs, get stopped by Skwovet", COLOR_WHITE);
    pbf_move_left_joystick(context, 128,   0, 3 * TICKS_PER_SECOND, 20);
    pbf_move_left_joystick(context,   0, 128, 3 * TICKS_PER_SECOND, 20);
    pbf_move_left_joystick(context, 128, 255, 3 * TICKS_PER_SECOND, 20);
    clear_dialog(console, context, ClearDialogMode::STOP_TIMEOUT, 5);

    context.wait_for_all_requests();
    console.log("Go to the kitchen, talk with mom");
    console.overlay().add_log("Go to the kitchen, talk with mom", COLOR_WHITE);
    pbf_move_left_joystick(context, 128, 255, 2 * TICKS_PER_SECOND, 20);
    overworld_navigation(env.program_info(), console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 0, 128);
    clear_dialog(console, context, ClearDialogMode::STOP_TIMEOUT, 5);

    context.wait_for_all_requests();
    console.log("Go to the front door, talk with Clavell");
    console.overlay().add_log("Go to the front door, talk with Clavell", COLOR_WHITE);
    pbf_move_left_joystick(context, 230, 200, 2 * TICKS_PER_SECOND, 20);
    overworld_navigation(env.program_info(), console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 255, 128);
    clear_dialog(console, context, ClearDialogMode::STOP_TIMEOUT, 5);

    context.wait_for_all_requests();
    console.log("Go upstairs, dress up");
    console.overlay().add_log("Go upstairs, dress up", COLOR_WHITE);
    pbf_move_left_joystick(context, 255, 128, 2 * TICKS_PER_SECOND, 20);
    pbf_move_left_joystick(context, 185,  10, 1 * TICKS_PER_SECOND, 20);
    pbf_move_left_joystick(context, 128,   0, 4 * TICKS_PER_SECOND, 20);
    pbf_move_left_joystick(context, 255, 128, 4 * TICKS_PER_SECOND, 20);
    pbf_move_left_joystick(context, 110, 200, 3 * TICKS_PER_SECOND, 20);
    pbf_move_left_joystick(context, 255, 128, 2 * TICKS_PER_SECOND, 20);
    // TODO: Tutorial detection
    pbf_mash_button(context, BUTTON_A, 20 * TICKS_PER_SECOND);

    context.wait_for_all_requests();
    console.log("Go to the living room, talk with Clavell");
    console.overlay().add_log("Go to the living room, talk with Clavell", COLOR_WHITE);
    pbf_move_left_joystick(context,   0,   0, 3 * TICKS_PER_SECOND, 20);
    pbf_move_left_joystick(context,   0, 128, 3 * TICKS_PER_SECOND, 20);
    pbf_move_left_joystick(context, 128, 255, 4 * TICKS_PER_SECOND, 20);
    overworld_navigation(env.program_info(), console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 0, 128);
    clear_dialog(console, context, ClearDialogMode::STOP_TIMEOUT, 10);

    context.wait_for_all_requests();
    console.log("Go outside, receive Rotom Phone");
    console.overlay().add_log("Go outside, receive Rotom Phone", COLOR_WHITE);
    overworld_navigation(env.program_info(), console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 245, 230);
    clear_dialog(console, context, ClearDialogMode::STOP_OVERWORLD);

    context.wait_for_all_requests();
    console.log("Bump into power of science NPC");
    console.overlay().add_log("Bump into power of science NPC", COLOR_WHITE);
    pbf_move_left_joystick(context, 128,   0, 33 * TICKS_PER_SECOND, 20);

    context.wait_for_all_requests();
    console.log("Clear map tutorial");
    console.overlay().add_log("Clear map tutorial", COLOR_WHITE);
    open_map_from_overworld(env.program_info(), console, context, true);
    leave_phone_to_overworld(env.program_info(), console, context);


}

void AutoStory::segment_01_01(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
    AutoStory_Descriptor::Stats& stats = env.current_stats<AutoStory_Descriptor::Stats>();
    while (true){   
        pbf_move_left_joystick(context, 255, 0, 1 * TICKS_PER_SECOND, 20);
        realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 155, 1 * TICKS_PER_SECOND);
        if (!overworld_navigation(env.program_info(), console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0)){
            context.wait_for_all_requests();
            console.log("Did not enter Nemona's house, resetting from checkpoint...", COLOR_RED);
            console.overlay().add_log("Failed to enter house, reset", COLOR_RED);
            reset_game(env.program_info(), console, context, "Did not enter Nemona's house, resetting from checkpoint...");
            stats.m_reset++;
            env.update_stats();
            continue;
        }
        context.wait_for_all_requests();
        console.overlay().add_log("Entered Nemona's house", COLOR_WHITE);
        mash_button_till_overworld(console, context);
        context.wait_for_all_requests();
        console.log("Picking a starter...");
        console.overlay().add_log("Picking a starter", COLOR_WHITE);
        switch(STARTERCHOICE){
        case StarterChoice::SPRIGATITO:
            console.log("Picking Sprigatito...");
            console.overlay().add_log("Picking Sprigatito...", COLOR_WHITE);
            pbf_move_left_joystick(context, 75, 0, 80, 20);
            break;
        case StarterChoice::FUECOCO:
            console.log("Picking Fuecoco...");
            console.overlay().add_log("Picking Fuecoco...", COLOR_WHITE);
            pbf_move_left_joystick(context, 180, 0, 80, 20);
            break;
        case StarterChoice::QUAXLY:
            console.log("Picking Quaxly...");
            console.overlay().add_log("Picking Quaxly...", COLOR_WHITE);
            pbf_move_left_joystick(context, 128, 0, 80, 20);
            break;
        }
        pbf_press_button(context, BUTTON_A, 20, 105);
        if (!clear_dialog(console, context, ClearDialogMode::STOP_PROMPT, 20)){
            context.wait_for_all_requests();
            console.log("Failed to pick starter, resetting from checkpoint...", COLOR_RED);
            console.overlay().add_log("Failed to select a starter, reset", COLOR_RED);
            reset_game(env.program_info(), console, context, "Failed to pick starter, resetting from checkpoint...");
            stats.m_reset++;
            env.update_stats();
            continue;
        }
        pbf_press_button(context, BUTTON_A, 20, 105);
        // Don't give a nickname
        if (!clear_dialog(console, context, ClearDialogMode::STOP_PROMPT, 20)){
            context.wait_for_all_requests();
            console.log("Stuck trying to give a nickname, resetting from checkpoint...", COLOR_RED);
            console.overlay().add_log("Stuck on nickname page, reset", COLOR_RED);
            reset_game(env.program_info(), console, context, "Stuck trying to give a nickname, resetting from checkpoint...");
            stats.m_reset++;
            env.update_stats();
            continue;
        }
        pbf_press_dpad(context, DPAD_DOWN,  20, 105);
        pbf_press_button(context, BUTTON_A, 20, 105);
        if (!clear_dialog(console, context, ClearDialogMode::STOP_OVERWORLD, 20)){
            context.wait_for_all_requests();
            console.log("Stuck trying to give a nickname, resetting from checkpoint...", COLOR_RED);
            console.overlay().add_log("Stuck on nickname page, reset", COLOR_RED);
            reset_game(env.program_info(), console, context, "Stuck trying to give a nickname, resetting from checkpoint...");
            stats.m_reset++;
            env.update_stats();
            continue;
        }
        break;
    }

    context.wait_for_all_requests();
    console.log("Clear auto heal tutorial");
    console.overlay().add_log("Clear auto heal tutorial", COLOR_WHITE);
    enter_menu_from_overworld(env.program_info(), console, context, 0, MenuSide::LEFT);
    pbf_press_button(context, BUTTON_A, 20, 8 * TICKS_PER_SECOND);

    context.wait_for_all_requests();
    console.log("Changing move order...");
    console.overlay().add_log("Changing move order...", COLOR_WHITE);
    // TODO: Detect move swapping
    pbf_press_dpad(context, DPAD_RIGHT, 15, 1 * TICKS_PER_SECOND);
    pbf_press_button(context, BUTTON_Y, 20, 40);
    pbf_press_button(context, BUTTON_A, 20, 40);
    pbf_press_dpad(context, DPAD_DOWN,  15, 40);
    pbf_press_dpad(context, DPAD_DOWN,  15, 40);
    pbf_press_button(context, BUTTON_A, 20, 40);
    leave_box_system_to_overworld(env.program_info(), console, context);    

}

void AutoStory::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    AutoStory_Descriptor::Stats& stats = env.current_stats<AutoStory_Descriptor::Stats>();

    // Connect controller
    pbf_press_button(context, BUTTON_L, 20, 20);

    switch (STARTPOINT){
    case StartPoint::INTRO_CUTSCENE:
        context.wait_for_all_requests();
        env.console.log("Start Segment 00: Intro Cutscene", COLOR_ORANGE);
        env.console.overlay().add_log("Start Segment 00: Intro Cutscene", COLOR_ORANGE);

        //TODO: Automate character settings

        // Mash A through intro cutscene
        // TODO: Stand up icon detection
        pbf_mash_button(context, BUTTON_A, 225 * TICKS_PER_SECOND);

        context.wait_for_all_requests();
        env.console.log("End Segment 00: Intro Cutscene", COLOR_GREEN);
        env.console.overlay().add_log("End Segment 00: Intro Cutscene", COLOR_GREEN);
        stats.m_segment++;
        env.update_stats();
        pbf_wait(context, 1 * TICKS_PER_SECOND);
    case StartPoint::IN_ROOM:
        segment_01_00(env, env.console, context);
        checkpoint_save(env, env.console, context);
        segment_01_01(env, env.console, context);
        checkpoint_save(env, env.console, context);

        context.wait_for_all_requests();
        env.console.log("End Segment 02: Pick Starter", COLOR_GREEN);
        env.console.overlay().add_log("End Segment 02: Pick Starter", COLOR_GREEN);
        stats.m_segment++;
        env.update_stats();
        if (ENDPOINT == EndPoint::IN_ROOM){
            break;
        }

    case StartPoint::NEMONA_FIRST_BATTLE:
        context.wait_for_all_requests();
        env.console.log("Start Segment 02: First Nemona Battle", COLOR_ORANGE);
        env.console.overlay().add_log("Start Segment 02: First Nemona Battle", COLOR_ORANGE);

        while (true){
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 220, 245, 50);
            pbf_move_left_joystick(context, 128, 0, 4 * TICKS_PER_SECOND, 1 * TICKS_PER_SECOND);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 128, 50);
            pbf_move_left_joystick(context, 128, 0, 4 * TICKS_PER_SECOND, 1 * TICKS_PER_SECOND);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 60, 50);
            pbf_move_left_joystick(context, 128, 0, 4 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
            if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_SPAM_A, 128, 0, 8)){
                context.wait_for_all_requests();
                env.console.log("Did not talk to Nemona at beach, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Can't find Nemona, reset", COLOR_RED);
                reset_game(env.program_info(), env.console, context, "Did not talk to Nemona at beach, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            context.wait_for_all_requests();
            env.console.overlay().add_log("Found Nemona", COLOR_WHITE);
            break;
        }

        context.wait_for_all_requests();
        env.console.log("Starting battle...");
        env.console.overlay().add_log("Starting battle...", COLOR_WHITE);
        // TODO: Battle start prompt detection
        mash_button_till_overworld(env.console, context);
        context.wait_for_all_requests();
        env.console.log("Finished battle.");
        env.console.overlay().add_log("Finished battle.", COLOR_WHITE);

        save_game_from_overworld(env.program_info(), env.console, context);
        stats.m_checkpoint++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE, "Saved at checkpoint.");

        context.wait_for_all_requests();
        env.console.log("End Segment 02: First Nemona Battle", COLOR_GREEN);
        env.console.overlay().add_log("End Segment 02: First Nemona Battle", COLOR_GREEN);
        stats.m_segment++;
        env.update_stats();
        if (ENDPOINT == EndPoint::NEMONA_FIRST_BATTLE){
            break;
        }

    case StartPoint::CATCH_TUTORIAL:
        context.wait_for_all_requests();
        env.console.log("Start Segment 03: Catch Tutorial", COLOR_ORANGE);
        env.console.overlay().add_log("Start Segment 03: Catch Tutorial", COLOR_ORANGE);

        while (true){
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 40, 160, 60);
            pbf_move_left_joystick(context, 128, 0, 7 * TICKS_PER_SECOND, 20);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 40, 84, 60);
            if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0, 20)){
                context.wait_for_all_requests();
                env.console.log("Did not find Mom, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Can't find mom, reset", COLOR_RED);
                reset_game(env.program_info(), env.console, context, "Did not find Mom, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            context.wait_for_all_requests();
            env.console.log("Get mom's sandwich");
            env.console.overlay().add_log("Get mom's sandwich", COLOR_WHITE);
            mash_button_till_overworld(env.console, context);
            break;
        }

        save_game_from_overworld(env.program_info(), env.console, context);
        stats.m_checkpoint++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE, "Saved at checkpoint.");

        while (true){
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 40, 82, 60);
            pbf_move_left_joystick(context, 128, 0, 6 * TICKS_PER_SECOND, 20);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 110, 10, 60);
            if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0, 20)){
                context.wait_for_all_requests();
                env.console.log("Did not find Nemona, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Can't find Nemona, reset", COLOR_RED);
                reset_game(env.program_info(), env.console, context, "Did not find Nemona, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            context.wait_for_all_requests();
            env.console.log("Start catch tutorial");
            env.console.overlay().add_log("Start catch tutorial", COLOR_WHITE);
            clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10);
            // TODO: Tutorial detection
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_button(context, BUTTON_A, 20, 105);
            run_battle(env.console, context, BattleStopCondition::STOP_DIALOG);
            clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10);
            // TODO: Tutorial detection
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_button(context, BUTTON_A, 20, 105);
            context.wait_for_all_requests();
            env.console.log("Finished catch tutorial");
            env.console.overlay().add_log("Finished catch tutorial", COLOR_WHITE);
            break;
        }

        save_game_from_overworld(env.program_info(), env.console, context);
        stats.m_checkpoint++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE, "Saved at checkpoint.");

        context.wait_for_all_requests();
        env.console.log("Move to cliff");
        env.console.overlay().add_log("Move to cliff", COLOR_WHITE);

        while (true){
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 240, 60, 80);
            if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 116, 0, 72, 24)){
                context.wait_for_all_requests();
                env.console.log("Did not reach cliff, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Did not reach cliff, reset", COLOR_RED);
                reset_game(env.program_info(), env.console, context, "Did not reach cliff, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            if (!clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD)){
                context.wait_for_all_requests();
                env.console.log("Did not reach cliff, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Did not reach cliff, reset", COLOR_RED);
                reset_game(env.program_info(), env.console, context, "Did not reach cliff, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            context.wait_for_all_requests();
            env.console.log("Mystery cry");
            env.console.overlay().add_log("Mystery cry", COLOR_WHITE);
            break;
        }

        save_game_from_overworld(env.program_info(), env.console, context);
        stats.m_checkpoint++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE, "Saved at checkpoint.");

        context.wait_for_all_requests();
        env.console.log("End Segment 03: Catch Tutorial", COLOR_GREEN);
        env.console.overlay().add_log("End Segment 03: Catch Tutorial", COLOR_GREEN);
        stats.m_segment++;
        env.update_stats();
        if (ENDPOINT == EndPoint::CATCH_TUTORIAL){
            break;
        }

    case StartPoint::LEGENDARY_RESCUE:
        context.wait_for_all_requests();
        env.console.log("Start Segment 04: Rescue Legendary", COLOR_ORANGE);
        env.console.overlay().add_log("Start Segment 04: Rescue Legendary", COLOR_ORANGE);

        while (true){
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 230, 70, 100);
            if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0)){
                context.wait_for_all_requests();
                env.console.log("Did not reach cliff, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Did not reach cliff, reset", COLOR_RED);
                reset_game(env.program_info(), env.console, context, "Did not reach cliff, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 20);
            // long animation
            clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10);
            if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0, 30)){
                context.wait_for_all_requests();
                env.console.log("Did not reach legendary, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Did not reach legendary, reset", COLOR_RED);
                reset_game(env.program_info(), env.console, context, "Did not reach legendaryf, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10);

            // TODO: Bag menu navigation
            context.wait_for_all_requests();
            env.console.log("Feed mom's sandwich");
            env.console.overlay().add_log("Feed mom's sandwich", COLOR_WHITE);
            pbf_press_dpad(context, DPAD_UP, 20, 105);
            pbf_mash_button(context, BUTTON_A, 100);
            clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 25);
            // long animation
            clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10);

            // First Nemona cave conversation
            context.wait_for_all_requests();
            env.console.log("Enter cave");
            env.console.overlay().add_log("Enter cave", COLOR_WHITE);
            pbf_move_left_joystick(context, 128, 20, 10 * TICKS_PER_SECOND, 20);
            pbf_move_left_joystick(context, 150, 20, 1 * TICKS_PER_SECOND, 20);
            pbf_move_left_joystick(context, 128, 20, 8 * TICKS_PER_SECOND, 20);
            pbf_move_left_joystick(context, 150, 20, 2 * TICKS_PER_SECOND, 20);
            if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 20, 10)){
                context.wait_for_all_requests();
                env.console.log("Did not enter cave, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Did not enter cave, reset", COLOR_RED);
                reset_game(env.program_info(), env.console, context, "Did not enter cave, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10);

            // Legendary rock break
            context.wait_for_all_requests();
            env.console.log("Rock break");
            env.console.overlay().add_log("Rock break", COLOR_WHITE);
            pbf_move_left_joystick(context, 128, 20, 3 * TICKS_PER_SECOND, 20);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NO_MARKER, 230, 25, 30);
            pbf_move_left_joystick(context, 128, 0, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);

            // Houndour wave
            context.wait_for_all_requests();
            env.console.log("Houndour wave");
            env.console.overlay().add_log("Houndour wave", COLOR_WHITE);
            pbf_move_left_joystick(context, 128, 20, 4 * TICKS_PER_SECOND, 20);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NO_MARKER, 200, 15, 30);
            pbf_move_left_joystick(context, 128, 20, 10 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NO_MARKER, 200, 25, 20);
            pbf_move_left_joystick(context, 128, 20, 11 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NO_MARKER, 230, 25, 25);
            pbf_move_left_joystick(context, 128, 20, 6 * TICKS_PER_SECOND, 20 * TICKS_PER_SECOND);

            // Houndoom encounter
            context.wait_for_all_requests();
            env.console.log("Houndoom encounter");
            env.console.overlay().add_log("Houndoom encounter", COLOR_WHITE);
            pbf_move_left_joystick(context, 128, 20, 4 * TICKS_PER_SECOND, 20);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NO_MARKER, 245, 20, 20);
            pbf_move_left_joystick(context, 128, 20, 2 * TICKS_PER_SECOND, 20);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NO_MARKER, 255, 90, 20);
            pbf_move_left_joystick(context, 128, 20, 8 * TICKS_PER_SECOND, 8 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_L, 20, 20);
            if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 20, 40)){
                context.wait_for_all_requests();
                env.console.log("Did not reach Houndoom, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Did not reach Houndoom, reset", COLOR_RED);
                reset_game(env.program_info(), env.console, context, "Did not reach Houndoom, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            mash_button_till_overworld(env.console, context, BUTTON_A);
            break;
        }

        save_game_from_overworld(env.program_info(), env.console, context);
        stats.m_checkpoint++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE, "Saved at checkpoint.");

        context.wait_for_all_requests();
        env.console.log("End Segment 04: Rescue Legendary", COLOR_GREEN);
        env.console.overlay().add_log("End Segment 04: Rescue Legendary", COLOR_GREEN);
        stats.m_segment++;
        env.update_stats();
        if (ENDPOINT == EndPoint::LEGENDARY_RESCUE){
            break;
        }

    case StartPoint::ARVEN_FIRST_BATTLE:
        context.wait_for_all_requests();
        env.console.log("Start Segment 05: First Arven Battle", COLOR_ORANGE);
        env.console.overlay().add_log("Start Segment 05: First Arven Battle", COLOR_ORANGE);

        while (true){
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 230, 120, 100);
            if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0)){
                context.wait_for_all_requests();
                env.console.log("Did not talk to Arven at lab, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Can't find Arven, reset", COLOR_RED);
                reset_game(env.program_info(), env.console, context, "Did not talk to Arven at lab, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            context.wait_for_all_requests();
            env.console.log("Found Arven");
            env.console.overlay().add_log("Found Arven", COLOR_WHITE);
            mash_button_till_overworld(env.console, context, BUTTON_A);
            context.wait_for_all_requests();
            env.console.log("Receive legendary ball");
            env.console.overlay().add_log("Receive legendary ball", COLOR_WHITE);
            break;
        }

        save_game_from_overworld(env.program_info(), env.console, context);
        stats.m_checkpoint++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE, "Saved at checkpoint.");

        while (true){
            context.wait_for_all_requests();
            env.console.log("Lighthouse view");
            env.console.overlay().add_log("Lighthouse view", COLOR_WHITE);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 230, 110, 100);
            pbf_move_left_joystick(context, 128, 0, 6 * TICKS_PER_SECOND, 8 * TICKS_PER_SECOND);
            pbf_move_left_joystick(context, 128, 0, 4 * TICKS_PER_SECOND, 20);
            pbf_move_left_joystick(context, 255, 128, 15, 20);
            pbf_press_button(context, BUTTON_L, 20, 20);
            pbf_move_left_joystick(context, 128, 0, 7 * TICKS_PER_SECOND, 20);
            if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_SPAM_A, 128, 0, 20)){
                context.wait_for_all_requests();
                env.console.log("Did not talk to Nemona on the lighthouse, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Can't find Nemona, reset", COLOR_RED);
                reset_game(env.program_info(), env.console, context, "Did not talk to Nemona on the lighthouse, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            mash_button_till_overworld(env.console, context, BUTTON_A);
            break;
        }

        save_game_from_overworld(env.program_info(), env.console, context);
        stats.m_checkpoint++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE, "Saved at checkpoint.");

        context.wait_for_all_requests();
        env.console.log("End Segment 05: First Arven Battle", COLOR_GREEN);
        env.console.overlay().add_log("End Segment 05: First Arven Battle", COLOR_GREEN);
        stats.m_segment++;
        env.update_stats();
        if (ENDPOINT == EndPoint::ARVEN_FIRST_BATTLE){
            break;
        }

    case StartPoint::LOS_PLATOS:
        context.wait_for_all_requests();
        env.console.log("Start Segment 06: Go to Los Platos", COLOR_ORANGE);
        env.console.overlay().add_log("Start Segment 06: Go to Los Platos", COLOR_ORANGE);

        while (true){
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 100, 210, 100);
            pbf_move_left_joystick(context, 128, 0, 187, 20);
            pbf_move_left_joystick(context, 0, 128, 30, 8 * TICKS_PER_SECOND);
            pbf_move_left_joystick(context, 128, 0, 1 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);

            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 100, 60, 200);
            if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0, 75)){
                context.wait_for_all_requests();
                env.console.log("Did not reach Los Platos, resetting from checkpoint...", COLOR_RED);
                env.console.overlay().add_log("Did not reach Los Platos, reset", COLOR_RED);
                reset_game(env.program_info(), env.console, context, "Did not reach Los Platos, resetting from checkpoint...");
                stats.m_reset++;
                env.update_stats();
                continue;
            }
            clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10);
            // TODO: Tutorial detection
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_button(context, BUTTON_A, 20, 105);
            context.wait_for_all_requests();
            env.console.log("Reached Los Platos");
            env.console.overlay().add_log("Reached Los Platos", COLOR_WHITE);
            break;
        }

        save_game_from_overworld(env.program_info(), env.console, context);
        stats.m_checkpoint++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE, "Saved at checkpoint.");

        context.wait_for_all_requests();
        env.console.log("End Segment 06: Go to Los Platos", COLOR_GREEN);
        env.console.overlay().add_log("End Segment 06: Go to Los Platos", COLOR_GREEN);
        stats.m_segment++;
        env.update_stats();
        if (ENDPOINT == EndPoint::LOS_PLATOS){
            break;
        }
    case StartPoint::MESAGOZA_SOUTH:
        context.wait_for_all_requests();
        env.console.log("Start Segment 07: Go to Mesagoza South", COLOR_ORANGE);
        env.console.overlay().add_log("Start Segment 07: Go to Mesagoza South", COLOR_ORANGE);

        // // Mystery Gift, delete later
        // enter_menu_from_overworld(env.program_info(), env.console, context, 2);
        // pbf_press_button(context, BUTTON_A, 20, 4 * TICKS_PER_SECOND);
        // pbf_press_dpad(context, DPAD_UP, 20, 105);
        // pbf_press_button(context, BUTTON_A, 20, 4 * TICKS_PER_SECOND);
        // pbf_press_dpad(context, DPAD_DOWN, 20, 105);
        // pbf_press_button(context, BUTTON_A, 20, 4 * TICKS_PER_SECOND);
        // pbf_press_button(context, BUTTON_A, 20, 10 * TICKS_PER_SECOND);
        // clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10);

        context.wait_for_all_requests();
        env.console.log("End Segment 07: Go to Mesagoza South", COLOR_GREEN);
        env.console.overlay().add_log("End Segment 07: Go to Mesagoza South", COLOR_GREEN);
        stats.m_segment++;
        env.update_stats();
        if (ENDPOINT == EndPoint::MESAGOZA_SOUTH){
            break;
        }

    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}






}
}
}
