/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <atomic>
#include <sstream>
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_SnapshotDumper.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/Battles/PokemonSV_Battles.h"
#include "PokemonSV/Programs/AutoStory/PokemonSV_MenuOption.h"
#include "PokemonSV/Programs/AutoStory/PokemonSV_TutorialDetector.h"
#include "PokemonSV/Programs/AutoStory/PokemonSV_PokemonMovesReader.h"
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
    : LANGUAGE(
        "<b>Game Language:</b>",
        PokemonSwSh::IV_READER().languages(),
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , STARTPOINT(
        "<b>Start Point:</b><br>Program will start with this segment.",
        {
            {StartPoint::INTRO_CUTSCENE,        "00_gameintro",         "00: Intro Cutscene"},
            {StartPoint::PICK_STARTER,          "01_pickstarter",       "01: Pick Starter"},
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
            {EndPoint::PICK_STARTER,            "01_pickstarter",       "01: Pick Starter"},
            {EndPoint::NEMONA_FIRST_BATTLE,     "02_nemonabattle1",     "02: First Nemona Battle"},
            {EndPoint::CATCH_TUTORIAL,          "03_catchtutorial",     "03: Catch Tutorial"},
            {EndPoint::LEGENDARY_RESCUE,        "04_legendaryrescue",   "04: Rescue Legendary"},
            {EndPoint::ARVEN_FIRST_BATTLE,      "05_arvenbattle1",      "05: First Arven Battle"},
            {EndPoint::LOS_PLATOS,              "06_losplatos",         "06: Go to Los Platos"},
            {EndPoint::MESAGOZA_SOUTH,          "07_mesagozasouth",     "07: Go to Mesagoza South"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        EndPoint::PICK_STARTER
    )
    , START_DESCRIPTION(
        ""
    )
    , END_DESCRIPTION(
        ""
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
    , m_advanced_options(
        "<font size=4><b>Advanced Options: (developer only)</b></font>"
    )
    , CHANGE_SETTINGS(
        "<b>Change settings at Program Start:</b><br>"
        "This is to ensure the program has the correct settings, particularly with Autosave turned off.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )    
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(STARTPOINT);
    PA_ADD_OPTION(START_DESCRIPTION);
    PA_ADD_OPTION(ENDPOINT);
    PA_ADD_OPTION(END_DESCRIPTION);
    PA_ADD_OPTION(STARTERCHOICE);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);

    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(m_advanced_options);
        PA_ADD_OPTION(CHANGE_SETTINGS);
    }

    AutoStory::value_changed(this);

    STARTPOINT.add_listener(*this);
    ENDPOINT.add_listener(*this);
}

void AutoStory::value_changed(void* object){
    ConfigOptionState state = (STARTPOINT == StartPoint::INTRO_CUTSCENE) || (STARTPOINT == StartPoint::PICK_STARTER)
        ? ConfigOptionState::ENABLED
        : ConfigOptionState::HIDDEN;
    STARTERCHOICE.set_visibility(state);

    START_DESCRIPTION.set_text(start_segment_description());
    END_DESCRIPTION.set_text(end_segment_description());
}

std::string AutoStory::start_segment_description(){
    switch(STARTPOINT){
    case StartPoint::INTRO_CUTSCENE:
        return "Start: Intro cutscene.";
    case StartPoint::PICK_STARTER:
        return "Start: Finished cutscene. Adjusted settings. Standing in room.";
    case StartPoint::NEMONA_FIRST_BATTLE:
        return "Start: Picked the starter.";
    case StartPoint::CATCH_TUTORIAL:
        return "Start: Battled Nemona on the beach.";
    case StartPoint::LEGENDARY_RESCUE:
        return "Start: Finished catch tutorial. Walked to the cliff.";
    case StartPoint::ARVEN_FIRST_BATTLE:
        return "Start: Saved the Legendary. Escaped from the Houndoom cave.";
    case StartPoint::LOS_PLATOS:
        return "Start: Battled Arven, received Legendary's Pokeball. Talked to Nemona at Lighthouse.";
    case StartPoint::MESAGOZA_SOUTH:
        return "Start: Reached Pokecenter at Los Platos.";
    default:
        return "";        
    }
}

std::string AutoStory::end_segment_description(){
    switch(ENDPOINT){
    case EndPoint::INTRO_CUTSCENE:
        return "End: Finished cutscene. Adjusted settings. Standing in room.";
    case EndPoint::PICK_STARTER:
        return "End: Picked the starter.";
    case EndPoint::NEMONA_FIRST_BATTLE:
        return "End: Battled Nemona on the beach.";
    case EndPoint::CATCH_TUTORIAL:
        return "End: Finished catch tutorial. Walked to the cliff.";
    case EndPoint::LEGENDARY_RESCUE:
        return "End: Saved the Legendary. Escaped from the Houndoom cave.";
    case EndPoint::ARVEN_FIRST_BATTLE:
        return "End: Battled Arven, received Legendary's Pokeball. Talked to Nemona at Lighthouse.";
    case EndPoint::LOS_PLATOS:
        return "End: Reached Pokecenter at Los Platos.";
    case EndPoint::MESAGOZA_SOUTH:
        return "End: ";
    default:
        return "";
    }
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
    int16_t num_times_seen_overworld = 0;
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
            num_times_seen_overworld++;
            if (stop_condition == BattleStopCondition::STOP_OVERWORLD){
                return true;
            }
            if(num_times_seen_overworld > 30){
                console.log("run_battle: Stuck in overworld.");
                return false;
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

void clear_tutorial(ConsoleHandle& console, BotBaseContext& context, uint16_t seconds_timeout){
    bool seen_tutorial = false;
    while (true){
        TutorialWatcher tutorial;
        context.wait_for_all_requests();

        int ret = wait_until(
            console, context,
            std::chrono::seconds(seconds_timeout),
            {tutorial}
        );
        context.wait_for(std::chrono::milliseconds(100));

        switch (ret){
        case 0:
            console.log("clear_tutorial: Detected tutorial screen.");
            seen_tutorial = true;
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        default:
            console.log("clear_tutorial: Timed out.");
            if(!seen_tutorial){
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT, console,
                    "clear_tutorial(): Tutorial screen never detected.",
                    true
                );                
            }
            return;
        }
    }
}

bool clear_dialog(ConsoleHandle& console, BotBaseContext& context,
    ClearDialogMode mode, uint16_t seconds_timeout
){
    bool seen_dialog = false;
    int16_t num_times_seen_overworld = 0;
    while (true){
        OverworldWatcher    overworld(COLOR_CYAN);
        PromptDialogWatcher prompt(COLOR_YELLOW);
        WhiteButtonWatcher  whitebutton(COLOR_GREEN, WhiteButton::ButtonA2, {0.725, 0.833, 0.024, 0.045}); // {0.650, 0.650, 0.140, 0.240}
        AdvanceDialogWatcher    advance_dialog(COLOR_RED);
        context.wait_for_all_requests();

        int ret = wait_until(
            console, context,
            std::chrono::seconds(seconds_timeout),
            {overworld, prompt, whitebutton, advance_dialog}
        );
        context.wait_for(std::chrono::milliseconds(100));

        switch (ret){
        case 0: // overworld
            console.log("clear_dialog: Detected overworld.");
            num_times_seen_overworld++;
            if (seen_dialog && mode == ClearDialogMode::STOP_OVERWORLD){
                return true;
            }
            if(num_times_seen_overworld > 30){
                console.log("clear_dialog: Stuck in overworld.");
                return false;
            }

            break;
        case 1: // prompt
            console.log("clear_dialog: Detected prompt.");
            seen_dialog = true;
            if (mode == ClearDialogMode::STOP_PROMPT){
                return true;
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case 2: // whitebutton
            console.log("clear_dialog: Detected white A button.");
            seen_dialog = true;
            if (mode == ClearDialogMode::STOP_WHITEBUTTON){
                return true;
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            // dump_snapshot(console);
            break;
        case 3: // advance dialog
            console.log("clear_dialog: Detected advance dialog.");
            seen_dialog = true;
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        default:
            console.log("clear_dialog(): Timed out.");
            if (seen_dialog && mode == ClearDialogMode::STOP_TIMEOUT){
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
            console.log("overworld_navigation: Detected start of battle.");
            run_battle(console, context, BattleStopCondition::STOP_OVERWORLD);
            auto_heal_from_menu_or_overworld(info, console, context, 0, true);
            realign_player(info, console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            break;
        case 1: // dialog
            console.log("overworld_navigation: Detected dialog.");
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


void config_option(BotBaseContext& context, int change_option_value){
    for (int i = 0; i < change_option_value; i++){
        pbf_press_dpad(context, DPAD_RIGHT, 15, 20);
    }
    pbf_press_dpad(context, DPAD_DOWN,  15, 20);
}

// NOTE: we can't confirm that the moves are actually swapped, unless we detect the moves themselves (either names or PP)
void swap_starter_moves(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, Language language){
    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::minutes(3)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "swap_starter_moves(): Failed to swap the starter moves after 3 minutes.",
                true
            );
        }
        // start in the overworld
        press_Bs_to_back_to_overworld(info, console, context);

        // open menu, select your starter
        enter_menu_from_overworld(info, console, context, 0, MenuSide::LEFT);

        // enter Pokemon summary screen
        pbf_press_button(context, BUTTON_A, 20, 5 * TICKS_PER_SECOND);
        pbf_press_dpad(context, DPAD_RIGHT, 15, 1 * TICKS_PER_SECOND);
        pbf_press_button(context, BUTTON_Y, 20, 40);

        // select move 1
        pbf_press_button(context, BUTTON_A, 20, 40);  
        pbf_press_dpad(context, DPAD_DOWN,  15, 40);
        pbf_press_dpad(context, DPAD_DOWN,  15, 40);
        // extra button presses to avoid drops
        pbf_press_dpad(context, DPAD_DOWN,  15, 40); 
        pbf_press_dpad(context, DPAD_DOWN,  15, 40);

        // select move 3. swap move 1 and move 3.
        pbf_press_button(context, BUTTON_A, 20, 40);    

        // confirm that Ember/Leafage/Water Gun is in slot 1
        if (language == Language::English){
            context.wait_for_all_requests();
            VideoSnapshot screen = console.video().snapshot();
            PokemonMovesReader reader(Language::English);
            std::string top_move = reader.read_move(console, screen, 0);
            console.log("Current top move: " + top_move);
            if (top_move != "ember" && top_move != "leafage" && top_move != "water-gun"){
                console.log("Failed to swap moves. Re-try.");
                continue;
            }   

        }

        break;    
    }

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
            [has_minimap](BotBaseContext& context){
                for (int i = 0; i < 10; i++){
                    pbf_wait(context, 3 * TICKS_PER_SECOND);
                    if (!has_minimap){ 
                        // if no minimap, can't detect overworld, so repeatedly press X to cover for button drops
                        pbf_press_button(context, BUTTON_X, 20, 100);
                    }
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

void AutoStory::change_settings_prior_to_autostory(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    if (STARTPOINT == StartPoint::INTRO_CUTSCENE){
        return;
    }
    
    enter_options_menu_from_overworld(env, context);
    change_settings(env, context);
    if(STARTPOINT == StartPoint::PICK_STARTER){
        pbf_mash_button(context, BUTTON_B, 2 * TICKS_PER_SECOND);
    }else{
        press_Bs_to_back_to_overworld(env.program_info(), env.console, context);        
    }
}

void AutoStory::enter_options_menu_from_overworld(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    int8_t index = option_index();
    if (index < 0){
        return;
    }
    bool has_minimap = !(STARTPOINT == StartPoint::INTRO_CUTSCENE || STARTPOINT == StartPoint::PICK_STARTER);

    enter_menu_from_overworld(env.program_info(), env.console, context, index, MenuSide::RIGHT, has_minimap);

}


int8_t AutoStory::option_index(){
    switch(STARTPOINT){
    // case StartPoint::INTRO_CUTSCENE:
    case StartPoint::PICK_STARTER:
        return 0;
    case StartPoint::NEMONA_FIRST_BATTLE:
        return 1;
    case StartPoint::CATCH_TUTORIAL:
    case StartPoint::LEGENDARY_RESCUE:
    case StartPoint::ARVEN_FIRST_BATTLE:
    case StartPoint::LOS_PLATOS:
    case StartPoint::MESAGOZA_SOUTH:
        return 2;
    default:
        return -1;        
    }    
}

void AutoStory::change_settings(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    env.console.log("Update settings.");
    if (LANGUAGE == Language::English){
        MenuOption session(env.console, context, LANGUAGE);
    
        std::vector<std::pair<MenuOptionItemEnum, MenuOptionToggleEnum>> options = {
            {MenuOptionItemEnum::TEXT_SPEED, MenuOptionToggleEnum::FAST},
            {MenuOptionItemEnum::SKIP_MOVE_LEARNING, MenuOptionToggleEnum::ON},
            {MenuOptionItemEnum::SEND_TO_BOXES, MenuOptionToggleEnum::AUTOMATIC},
            {MenuOptionItemEnum::GIVE_NICKNAMES, MenuOptionToggleEnum::OFF},
            {MenuOptionItemEnum::VERTICAL_CAMERA_CONTROLS, MenuOptionToggleEnum::REGULAR},
            {MenuOptionItemEnum::HORIZONTAL_CAMERA_CONTROLS, MenuOptionToggleEnum::REGULAR},
            {MenuOptionItemEnum::CAMERA_SUPPORT, MenuOptionToggleEnum::ON},
            {MenuOptionItemEnum::CAMERA_INTERPOLATION, MenuOptionToggleEnum::NORMAL},
            {MenuOptionItemEnum::CAMERA_DISTANCE, MenuOptionToggleEnum::CLOSE},
            {MenuOptionItemEnum::AUTOSAVE, MenuOptionToggleEnum::OFF},
            {MenuOptionItemEnum::SHOW_NICKNAMES, MenuOptionToggleEnum::DONT_SHOW},
            {MenuOptionItemEnum::SKIP_CUTSCENES, MenuOptionToggleEnum::ON},
            {MenuOptionItemEnum::CONTROLLER_RUMBLE, MenuOptionToggleEnum::ON},
            {MenuOptionItemEnum::HELPING_FUNCTIONS, MenuOptionToggleEnum::OFF},

        };
        session.set_options(options);

    }else{
        //TODO: Add OCR file for other languages

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
    }

    pbf_mash_button(context, BUTTON_A, 1 * TICKS_PER_SECOND);
    clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 5);
    
}

void do_action_and_monitor_for_battles(
    ProgramEnvironment& env,
    ConsoleHandle& console, 
    BotBaseContext& context,
    std::function<
        void(ProgramEnvironment& env,
        ConsoleHandle& console,
        BotBaseContext& context)
    >&& action
){
    NormalBattleMenuWatcher battle_menu(COLOR_RED);
    int ret = run_until(
        console, context,
        [&](BotBaseContext& context){
            context.wait_for_all_requests();
            action(env, console, context);
        },
        {battle_menu}
    );
    if (ret == 0){ // battle detected
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "do_action_and_monitor_for_battles(): Detected battle. Failed to complete action.",
            true
        );

        // console.log("Detected battle. Now running away.", COLOR_PURPLE);
        // console.overlay().add_log("Detected battle. Now running away.");
        // try{
        //     run_from_battle(env.program_info(), console, context);
        // }catch (OperationFailedException& e){
        //     throw FatalProgramException(std::move(e));
        // }
    }
}

void AutoStory::checkpoint_save(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    AutoStory_Descriptor::Stats& stats = env.current_stats<AutoStory_Descriptor::Stats>();
    save_game_from_overworld(env.program_info(), env.console, context);
    stats.m_checkpoint++;
    env.update_stats();
    send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE, "Saved at checkpoint.");        
}

void AutoStory::test_segments(
    SingleSwitchProgramEnvironment& env,
    ConsoleHandle& console, 
    BotBaseContext& context,
    int start, int end, int loop
){
    int segment = start;
    while (segment <= end){
        switch(segment){
        case 0:
            segment_00(env, context);
            break;
        case 1:
            for (int i = 0; i < loop; i++){
                if (i > 0){
                    reset_game(env.program_info(), console, context);
                }
                console.log("segment_01: loop " + std::to_string(i));
                segment_01(env, context);
            }
            break;
        case 2:
            for (int i = 0; i < loop; i++){
                if (i > 0){
                    reset_game(env.program_info(), console, context);
                }
                console.log("segment_02: loop " + std::to_string(i));
                segment_02(env, context);
            }
            break;
        case 3:
            for (int i = 0; i < loop; i++){
                if (i > 0){
                    reset_game(env.program_info(), console, context);
                }
                console.log("segment_03: loop " + std::to_string(i));
                segment_03(env, context);
            }
            break;  
        case 4:
            for (int i = 0; i < loop; i++){
                if (i > 0){
                    reset_game(env.program_info(), console, context);
                }
                console.log("segment_04: loop " + std::to_string(i));
                segment_04(env, context);
            }
            break;
        case 5:
            for (int i = 0; i < loop; i++){
                if (i > 0){
                    reset_game(env.program_info(), console, context);
                }
                console.log("segment_05: loop " + std::to_string(i));
                segment_05(env, context);
            }
            break; 
        case 6:
            for (int i = 0; i < loop; i++){
                if (i > 0){
                    reset_game(env.program_info(), console, context);
                }
                console.log("segment_06: loop " + std::to_string(i));
                segment_06(env, context);
            }
            break;  
        case 7:
            for (int i = 0; i < loop; i++){
                if (i > 0){
                    reset_game(env.program_info(), console, context);
                }
                console.log("segment_07: loop " + std::to_string(i));
                segment_07(env, context);
            }
            break;
        case 8:
            for (int i = 0; i < loop; i++){
                if (i > 0){
                    reset_game(env.program_info(), console, context);
                }
                console.log("segment_08: loop " + std::to_string(i));
                segment_08(env, context);
            }
            break; 
        case 9:
            for (int i = 0; i < loop; i++){
                if (i > 0){
                    reset_game(env.program_info(), console, context);
                }
                console.log("segment_09: loop " + std::to_string(i));
                segment_09(env, context);
            }
            break; 
        case 10:
            for (int i = 0; i < loop; i++){
                if (i > 0){
                    reset_game(env.program_info(), console, context);
                }
                console.log("segment_10: loop " + std::to_string(i));
                segment_10(env, context);
            }
            break; 
        case 11:
            for (int i = 0; i < loop; i++){
                if (i > 0){
                    reset_game(env.program_info(), console, context);
                }
                console.log("segment_11: loop " + std::to_string(i));
                segment_11(env, context);
            }
            break;                                     

        }

        segment++;
    }
    
}

void AutoStory::segment_00(SingleSwitchProgramEnvironment& env, BotBaseContext& context){


    // Mash A through intro cutscene, until the L stick button is detected
    WhiteButtonWatcher leftstick(COLOR_GREEN, WhiteButton::ButtonLStick, {0.435, 0.912, 0.042, 0.047});
    context.wait_for_all_requests();
    run_until(
        env.console, context,
        [](BotBaseContext& context){
            pbf_mash_button(context, BUTTON_A, 240 * TICKS_PER_SECOND);
        },
        {leftstick}
    );
    
    // Stand up from chair and walk to left side of room
    pbf_move_left_joystick(context, 128, 255, 3 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
    pbf_move_left_joystick(context,   0, 128, 6 * TICKS_PER_SECOND, 1 * TICKS_PER_SECOND);

    // set settings
    enter_menu_from_overworld(env.program_info(), env.console, context, 0, MenuSide::RIGHT, false);
    change_settings(env, context);
    pbf_mash_button(context, BUTTON_B, 2 * TICKS_PER_SECOND);
}

void AutoStory::segment_01(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    AutoStory_Descriptor::Stats& stats = env.current_stats<AutoStory_Descriptor::Stats>();
    save_game_tutorial(env.program_info(), env.console, context);
    stats.m_checkpoint++;
    env.update_stats();
    send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE, "Saved at checkpoint.");     
    
    while (true){
    try{
        context.wait_for_all_requests();
        env.console.log("Go downstairs, get stopped by Skwovet");
        env.console.overlay().add_log("Go downstairs, get stopped by Skwovet", COLOR_WHITE);
        pbf_move_left_joystick(context, 128,   0, 3 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context,   0, 128, 3 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context, 128, 255, 3 * TICKS_PER_SECOND, 20);
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 5);

        context.wait_for_all_requests();
        env.console.log("Go to the kitchen, talk with mom");
        env.console.overlay().add_log("Go to the kitchen, talk with mom", COLOR_WHITE);
        pbf_move_left_joystick(context, 128, 255, 2 * TICKS_PER_SECOND, 20);
        if(!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 0, 128)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Failed to talk to mom.",
                true
            );
        }
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 5);

        context.wait_for_all_requests();
        env.console.log("Go to the front door, talk with Clavell");
        env.console.overlay().add_log("Go to the front door, talk with Clavell", COLOR_WHITE);
        pbf_move_left_joystick(context, 230, 200, 2 * TICKS_PER_SECOND, 20);
        if(!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 255, 128)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Failed to talk to Clavell.",
                true
            );            
        }
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 5);

        context.wait_for_all_requests();
        env.console.log("Go upstairs, dress up");
        env.console.overlay().add_log("Go upstairs, dress up", COLOR_WHITE);
        pbf_move_left_joystick(context, 255, 128, 2 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context, 185,  10, 1 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context, 128,   0, 4 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context, 255, 128, 4 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context, 110, 200, 3 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context, 255, 128, 2 * TICKS_PER_SECOND, 20);
        pbf_mash_button(context, BUTTON_A, 20 * TICKS_PER_SECOND);

        context.wait_for_all_requests();
        env.console.log("Go to the living room, talk with Clavell");
        env.console.overlay().add_log("Go to the living room, talk with Clavell", COLOR_WHITE);
        pbf_move_left_joystick(context,   0,   0, 3 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context,   0, 128, 3 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context, 128, 255, 4 * TICKS_PER_SECOND, 20);
        if(!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 0, 128)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Failed to talk to Clavell.",
                true
            );               
        }
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10);

        context.wait_for_all_requests();
        env.console.log("Go outside, receive Rotom Phone");
        env.console.overlay().add_log("Go outside, receive Rotom Phone", COLOR_WHITE);
        if(!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 245, 230)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Failed to go outside.",
                true
            );             
        }
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD);

        context.wait_for_all_requests();
        env.console.log("Bump into power of science NPC");
        // console.overlay().add_log("Bump into power of science NPC", COLOR_WHITE);
        pbf_move_left_joystick(context, 128,   0, 33 * TICKS_PER_SECOND, 20);

        context.wait_for_all_requests();
        env.console.log("Clear map tutorial");
        // console.overlay().add_log("Clear map tutorial", COLOR_WHITE);
        open_map_from_overworld(env.program_info(), env.console, context, true);
        leave_phone_to_overworld(env.program_info(), env.console, context);

        break;  
    }catch(OperationFailedException& e){
        context.wait_for_all_requests();
        env.console.log(e.m_message, COLOR_RED);
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }
    }
}

void AutoStory::segment_02(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    AutoStory_Descriptor::Stats& stats = env.current_stats<AutoStory_Descriptor::Stats>();
    checkpoint_save(env, context);
    
    while (true){   
    try{
        context.wait_for_all_requests();
        
        pbf_move_left_joystick(context, 255, 0, 1 * TICKS_PER_SECOND, 20);
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 156, 1 * TICKS_PER_SECOND);
        if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0)){
            // timed out before detecting dialog box
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Failed to enter Nemona's house.",
                true
            );               
        }
        context.wait_for_all_requests();
        env.console.log("Entered Nemona's house");
        mash_button_till_overworld(env.console, context);
        context.wait_for_all_requests();
        env.console.log("Picking a starter...");
        switch(STARTERCHOICE){
        case StarterChoice::SPRIGATITO:
            env.console.log("Picking Sprigatito...");
            pbf_move_left_joystick(context, 75, 0, 80, 20);
            break;
        case StarterChoice::FUECOCO:
            env.console.log("Picking Fuecoco...");
            pbf_move_left_joystick(context, 180, 0, 80, 20);
            break;
        case StarterChoice::QUAXLY:
            env.console.log("Picking Quaxly...");
            pbf_move_left_joystick(context, 128, 0, 80, 20);
            break;
        }
        pbf_press_button(context, BUTTON_A, 20, 105); // choose the starter
        if (!clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 20)){
            // timed out before detecting the dialog prompt, to confirm receiving the starter
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Failed to pick starter.",
                true
            );  
        }
        pbf_press_button(context, BUTTON_A, 20, 105); // accept the pokemon
        if (!clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 20)){
            // timed out before detecting the dialog prompt to give a nickname
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Stuck trying to give a nickname.",
                true
            );  
        }
        pbf_mash_button(context, BUTTON_B, 100);  // Don't give a nickname
        if (!clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 20)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Stuck trying to give a nickname.",
                true
            );  
        }

        context.wait_for_all_requests();
        env.console.log("Clear auto heal tutorial.");
        // Press X until Auto heal tutorial shows up
        TutorialWatcher tutorial;
        int ret = run_until(
            env.console, context,
            [](BotBaseContext& context){
                for (int i = 0; i < 10; i++){
                    pbf_press_button(context, BUTTON_X, 20, 250);
                }
            },
            {tutorial}
        );
        if (ret < 0){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Stuck trying to clear auto heal tutorial.",
                true
            );  
        }
        clear_tutorial(env.console, context);

        env.console.log("Change move order.");
        swap_starter_moves(env.program_info(), env.console, context, LANGUAGE);
        leave_box_system_to_overworld(env.program_info(), env.console, context);

        break;
    }catch(OperationFailedException& e){
        context.wait_for_all_requests();
        env.console.log(e.m_message, COLOR_RED);
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }
    }

}

void AutoStory::segment_03(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    AutoStory_Descriptor::Stats& stats = env.current_stats<AutoStory_Descriptor::Stats>();
    checkpoint_save(env, context);
    
    while (true){
    try{        
        context.wait_for_all_requests();

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 220, 245, 50);
        pbf_move_left_joystick(context, 128, 0, 4 * TICKS_PER_SECOND, 1 * TICKS_PER_SECOND);
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 128, 50);
        pbf_move_left_joystick(context, 128, 0, 4 * TICKS_PER_SECOND, 1 * TICKS_PER_SECOND);
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 60, 50);
        pbf_move_left_joystick(context, 128, 0, 4 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
        if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_SPAM_A, 128, 0, 8)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Failed to talk to Nemona at beach.",
                true
            );  
        }
        context.wait_for_all_requests();
        env.console.overlay().add_log("Found Nemona", COLOR_WHITE);

        context.wait_for_all_requests();
        env.console.log("Starting battle...");
        env.console.overlay().add_log("Starting battle...", COLOR_WHITE);
        // TODO: Battle start prompt detection
        mash_button_till_overworld(env.console, context);
        context.wait_for_all_requests();
        env.console.log("Finished battle.");
        env.console.overlay().add_log("Finished battle.", COLOR_WHITE);        

        break;
    }catch(OperationFailedException& e){
        context.wait_for_all_requests();
        env.console.log(e.m_message, COLOR_RED);
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }        
    }

}

void AutoStory::segment_04(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    AutoStory_Descriptor::Stats& stats = env.current_stats<AutoStory_Descriptor::Stats>();
    checkpoint_save(env, context);
    
    while (true){
    try{
        context.wait_for_all_requests();

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 40, 160, 60);
        pbf_move_left_joystick(context, 128, 0, 7 * TICKS_PER_SECOND, 20);
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 40, 84, 60);
        if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0, 20)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Failed to find Mom.",
                true
            );  
        }
        context.wait_for_all_requests();
        env.console.log("Get mom's sandwich");
        env.console.overlay().add_log("Get mom's sandwich", COLOR_WHITE);
        mash_button_till_overworld(env.console, context);
        break;
    }catch(OperationFailedException& e){
        context.wait_for_all_requests();
        env.console.log(e.m_message, COLOR_RED);
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }             
    }    
}

void AutoStory::segment_05(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    AutoStory_Descriptor::Stats& stats = env.current_stats<AutoStory_Descriptor::Stats>();
    checkpoint_save(env, context);
    context.wait_for_all_requests();
    while (true){
    try{        
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 40, 82, 60);
        pbf_move_left_joystick(context, 128, 0, 6 * TICKS_PER_SECOND, 20);
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 110, 10, 60);
        if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0, 20)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Failed to find Nemona.",
                true
            );  
        }
        context.wait_for_all_requests();
        env.console.log("Start catch tutorial");
        env.console.overlay().add_log("Start catch tutorial", COLOR_WHITE);
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10);
        clear_tutorial(env.console, context);
        run_battle(env.console, context, BattleStopCondition::STOP_DIALOG);
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 5);
        clear_tutorial(env.console, context);
        context.wait_for_all_requests();
        env.console.log("Finished catch tutorial");
        env.console.overlay().add_log("Finished catch tutorial", COLOR_WHITE);

        break;
    }catch(OperationFailedException& e){
        context.wait_for_all_requests();
        env.console.log(e.m_message, COLOR_RED);
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }            
    }
}

void AutoStory::segment_06(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    AutoStory_Descriptor::Stats& stats = env.current_stats<AutoStory_Descriptor::Stats>();
    checkpoint_save(env, context);

    while (true){
    try{
        context.wait_for_all_requests();
        env.console.log("Move to cliff");
        env.console.overlay().add_log("Move to cliff", COLOR_WHITE);

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 240, 60, 80);
        if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 116, 0, 72, 24)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Failed to reach cliff.",
                true
            );  
        }
        if (!clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Failed to reach cliff.",
                true
            );  
        }
        context.wait_for_all_requests();
        env.console.log("Mystery cry");
        env.console.overlay().add_log("Mystery cry", COLOR_WHITE);
        
        break;
    }catch(OperationFailedException& e){
        context.wait_for_all_requests();
        env.console.log(e.m_message, COLOR_RED);
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }           
    }
}

void AutoStory::segment_07(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    AutoStory_Descriptor::Stats& stats = env.current_stats<AutoStory_Descriptor::Stats>();
    checkpoint_save(env, context);

    while (true){
    try{
        context.wait_for_all_requests();

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 230, 70, 100);
        if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Failed to reach cliff.",
                true
            );  
        }
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 20);
        // long animation
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10);
        if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0, 30)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Failed to reach legendary.",
                true
            );  
        }
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10);

        // TODO: Bag menu navigation
        context.wait_for_all_requests();
        env.console.log("Feed mom's sandwich");
        env.console.overlay().add_log("Feed mom's sandwich", COLOR_WHITE);
        
        GradientArrowWatcher arrow(COLOR_RED, GradientArrowType::RIGHT, {0.104, 0.312, 0.043, 0.08});
        context.wait_for_all_requests();

        int ret = run_until(
            env.console, context,
            [](BotBaseContext& context){
                for (int i = 0; i < 10; i++){
                    pbf_press_dpad(context, DPAD_UP, 20, 250);
                }
            },
            {arrow}
        );
        if (ret < 0){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Failed to feed mom's sandwich.",
                true
            );  
        }

        // only press A when the sandwich is selected
        pbf_mash_button(context, BUTTON_A, 100);

        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 25);
        // long animation
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10);

        // First Nemona cave conversation
        context.wait_for_all_requests();
        env.console.log("Enter cave");
        env.console.overlay().add_log("Enter cave", COLOR_WHITE);
        do_action_and_monitor_for_battles(env, env.console, context,
            [&](ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
                pbf_move_left_joystick(context, 128, 20, 10 * TICKS_PER_SECOND, 20);
                pbf_move_left_joystick(context, 150, 20, 1 * TICKS_PER_SECOND, 20);
                pbf_move_left_joystick(context, 128, 20, 8 * TICKS_PER_SECOND, 20);
                pbf_move_left_joystick(context, 150, 20, 2 * TICKS_PER_SECOND, 20);                
            }
        );

        if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 20, 10)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Failed to enter cave.",
                true
            );  
        }
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10);

        do_action_and_monitor_for_battles(env, env.console, context,
            [&](ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
                // Legendary rock break
                context.wait_for_all_requests();
                console.log("Rock break");
                console.overlay().add_log("Rock break", COLOR_WHITE);
                pbf_move_left_joystick(context, 128, 20, 3 * TICKS_PER_SECOND, 20);
                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NO_MARKER, 230, 25, 30);
                pbf_move_left_joystick(context, 128, 0, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);

                // Houndour wave
                context.wait_for_all_requests();
                console.log("Houndour wave");
                console.overlay().add_log("Houndour wave", COLOR_WHITE);
                pbf_move_left_joystick(context, 130, 20, 4 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NO_MARKER, 220, 15, 30);
                pbf_move_left_joystick(context, 128, 20, 5 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
                pbf_move_left_joystick(context, 128, 20, 6 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NO_MARKER, 220, 25, 20);
                pbf_move_left_joystick(context, 128, 20, 8 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NO_MARKER, 220, 25, 25);
                pbf_move_left_joystick(context, 128, 20, 6 * TICKS_PER_SECOND, 20 * TICKS_PER_SECOND);

                // Houndoom encounter
                context.wait_for_all_requests();
                console.log("Houndoom encounter");
                console.overlay().add_log("Houndoom encounter", COLOR_WHITE);
                pbf_move_left_joystick(context, 128, 20, 4 * TICKS_PER_SECOND, 20);
                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NO_MARKER, 245, 20, 20);
                pbf_move_left_joystick(context, 128, 20, 2 * TICKS_PER_SECOND, 20);
                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NO_MARKER, 255, 90, 20);
                pbf_move_left_joystick(context, 128, 20, 8 * TICKS_PER_SECOND, 8 * TICKS_PER_SECOND);
                pbf_press_button(context, BUTTON_L, 20, 20);
            }
        );
        
        if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 20, 40)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Failed to reach Houndoom.",
                true
            );  
        }
        mash_button_till_overworld(env.console, context, BUTTON_A);

        break;
    }catch(OperationFailedException& e){
        context.wait_for_all_requests();
        env.console.log(e.m_message, COLOR_RED);
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }            
    }

}


void AutoStory::segment_08(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    AutoStory_Descriptor::Stats& stats = env.current_stats<AutoStory_Descriptor::Stats>();
    checkpoint_save(env, context);
    context.wait_for_all_requests();
    while (true){
    try{        
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 230, 120, 100);
        if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Failed to talk to Arven at the tower.",
                true
            );  
        }
        context.wait_for_all_requests();
        env.console.log("Found Arven");
        env.console.overlay().add_log("Found Arven", COLOR_WHITE);
        mash_button_till_overworld(env.console, context, BUTTON_A);
        context.wait_for_all_requests();
        env.console.log("Receive legendary ball");
        env.console.overlay().add_log("Receive legendary ball", COLOR_WHITE);

        break;
    }catch(OperationFailedException& e){
        context.wait_for_all_requests();
        env.console.log(e.m_message, COLOR_RED);
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }            
    }
}

void AutoStory::segment_09(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    AutoStory_Descriptor::Stats& stats = env.current_stats<AutoStory_Descriptor::Stats>();
    checkpoint_save(env, context);
    context.wait_for_all_requests();
    while (true){
    try{
        context.wait_for_all_requests();
        env.console.log("Lighthouse view");
        env.console.overlay().add_log("Lighthouse view", COLOR_WHITE);
        do_action_and_monitor_for_battles(env, env.console, context,
            [&](ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 230, 110, 100);
                pbf_move_left_joystick(context, 128, 0, 6 * TICKS_PER_SECOND, 8 * TICKS_PER_SECOND);
                pbf_move_left_joystick(context, 128, 0, 4 * TICKS_PER_SECOND, 20);
                pbf_move_left_joystick(context, 255, 128, 15, 20);
                pbf_press_button(context, BUTTON_L, 20, 20);
                pbf_move_left_joystick(context, 128, 0, 7 * TICKS_PER_SECOND, 20);                
            }
        );
        
        if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_SPAM_A, 128, 0, 20)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Failed to talk to Nemona on the lighthouse.",
                true
            );  
        }
        mash_button_till_overworld(env.console, context, BUTTON_A);

        break;
    }catch(OperationFailedException& e){
        context.wait_for_all_requests();
        env.console.log(e.m_message, COLOR_RED);
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }           
    }
}

void AutoStory::segment_10(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    AutoStory_Descriptor::Stats& stats = env.current_stats<AutoStory_Descriptor::Stats>();
    checkpoint_save(env, context);
    context.wait_for_all_requests();

    while (true){
    try{
        do_action_and_monitor_for_battles(env, env.console, context,
            [&](ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 100, 210, 100);
                pbf_move_left_joystick(context, 128, 0, 187, 20);
                pbf_move_left_joystick(context, 0, 128, 30, 8 * TICKS_PER_SECOND);
                pbf_move_left_joystick(context, 128, 0, 1 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);

                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 100, 60, 200);                
            }
        );     

        if (!overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0, 75)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Failed to reach Los Platos.",
                true
            );  
        }
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10);
        clear_tutorial(env.console, context);
        context.wait_for_all_requests();
        env.console.log("Reached Los Platos");
        env.console.overlay().add_log("Reached Los Platos", COLOR_WHITE);
        break;
    }catch(OperationFailedException& e){
        context.wait_for_all_requests();
        env.console.log(e.m_message, COLOR_RED);
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }             
    }

}


void AutoStory::segment_11(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    // AutoStory_Descriptor::Stats& stats = env.current_stats<AutoStory_Descriptor::Stats>();
    // checkpoint_save(env, env.console, context);
    context.wait_for_all_requests();

}

void AutoStory::run_autostory(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    AutoStory_Descriptor::Stats& stats = env.current_stats<AutoStory_Descriptor::Stats>();
    switch (STARTPOINT){
    case StartPoint::INTRO_CUTSCENE:
        context.wait_for_all_requests();
        env.console.log("Start Segment 00: Intro Cutscene", COLOR_ORANGE);
        env.console.overlay().add_log("Start Segment 00: Intro Cutscene", COLOR_ORANGE);

        segment_00(env, context);

        context.wait_for_all_requests();
        env.console.log("End Segment 00: Intro Cutscene", COLOR_GREEN);
        env.console.overlay().add_log("End Segment 00: Intro Cutscene", COLOR_GREEN);
        stats.m_segment++;
        env.update_stats();
        pbf_wait(context, 1 * TICKS_PER_SECOND);        
    case StartPoint::PICK_STARTER:
        context.wait_for_all_requests();
        env.console.log("Start Segment 01: Pick Starter", COLOR_ORANGE);
        env.console.overlay().add_log("Start Segment 01: Pick Starter", COLOR_ORANGE);

        segment_01(env, context);
        segment_02(env, context);

        context.wait_for_all_requests();
        env.console.log("End Segment 02: Pick Starter", COLOR_GREEN);
        env.console.overlay().add_log("End Segment 02: Pick Starter", COLOR_GREEN);
        stats.m_segment++;
        env.update_stats();
        if (ENDPOINT == EndPoint::PICK_STARTER){
            break;
        }

    case StartPoint::NEMONA_FIRST_BATTLE:
        context.wait_for_all_requests();
        env.console.log("Start Segment 02: First Nemona Battle", COLOR_ORANGE);
        env.console.overlay().add_log("Start Segment 02: First Nemona Battle", COLOR_ORANGE);

        segment_03(env, context);

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

        segment_04(env, context);
        segment_05(env, context);
        segment_06(env, context);

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

        segment_07(env, context);

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

        segment_08(env, context);
        segment_09(env, context);

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

        segment_10(env, context);

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
}

void AutoStory::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    // AutoStory_Descriptor::Stats& stats = env.current_stats<AutoStory_Descriptor::Stats>();

    // Connect controller
    pbf_press_button(context, BUTTON_L, 20, 20);

    // Set settings. to ensure autosave is off.
    // TODO: enable it for other languages
    if (LANGUAGE == Language::English && CHANGE_SETTINGS){
        change_settings_prior_to_autostory(env, context);
    }

    int start = 0;
    int end = 10;
    int loops = 10;
    test_segments(env, env.console, context, start, end, loops);


    // run_autostory(env, context);
    
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}






}
}
}
