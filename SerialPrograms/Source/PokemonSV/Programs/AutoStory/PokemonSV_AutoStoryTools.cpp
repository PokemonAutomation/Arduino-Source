/*  AutoStoryTools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_SnapshotDumper.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/Map/PokemonSV_MapMenuDetector.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/Battles/PokemonSV_Battles.h"
#include "PokemonSV/Programs/AutoStory/PokemonSV_MenuOption.h"
#include "PokemonSV/Inference/PokemonSV_TutorialDetector.h"
#include "PokemonSV/Inference/PokemonSV_PokemonMovesReader.h"
#include "PokemonSV/Inference/Map/PokemonSV_DestinationMarkerDetector.h"
#include "PokemonSV_AutoStoryTools.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;




void run_battle_press_A(
    ConsoleHandle& console, 
    BotBaseContext& context,
    BattleStopCondition stop_condition
){
    int16_t num_times_seen_overworld = 0;
    while (true){
        NormalBattleMenuWatcher battle(COLOR_BLUE);
        // SwapMenuWatcher         fainted(COLOR_PURPLE);
        OverworldWatcher        overworld(COLOR_CYAN);
        AdvanceDialogWatcher    dialog(COLOR_RED);
        context.wait_for_all_requests();

        int ret = wait_until(
            console, context,
            std::chrono::seconds(90),
            {battle, overworld, dialog}
        );
        context.wait_for(std::chrono::milliseconds(100));

        switch (ret){
        case 0: // battle
            console.log("Detected battle menu, spam first move.");
            pbf_mash_button(context, BUTTON_A, 3 * TICKS_PER_SECOND);
            break;
        case 1: // overworld
            console.log("Detected overworld, battle over.");
            num_times_seen_overworld++;
            if (stop_condition == BattleStopCondition::STOP_OVERWORLD){
                return;
            }
            if(num_times_seen_overworld > 30){
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT, console,
                    "run_battle_press_A(): Stuck in overworld. Did not detect expected stop condition.",
                    true
                );  
            }            
            break;
        case 2: // advance dialog
            console.log("Detected dialog.");
            {
                context.wait_for_all_requests();
                WipeoutDetector wipeout;
                VideoSnapshot screen = console.video().snapshot();
                // dump_snapshot(console);
                if (wipeout.detect(screen)){
                    throw OperationFailedException(
                        ErrorReport::SEND_ERROR_REPORT, console,
                        "run_battle_press_A(): Detected wipeout. All pokemon fainted.",
                        true
                    );                
                }
            }

            if (stop_condition == BattleStopCondition::STOP_DIALOG){
                return;
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        default: // timeout
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "run_battle_press_A(): Timed out. Did not detect expected stop condition.",
                true
            );             
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

void clear_dialog(ConsoleHandle& console, BotBaseContext& context,
    ClearDialogMode mode, uint16_t seconds_timeout,
    std::vector<ClearDialogCallback> enum_optional_callbacks
){
    bool seen_dialog = false;
    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::minutes(3)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "clear_dialog(): Failed to clear dialog after 3 minutes.",
                true
            );
        }

        AdvanceDialogWatcher    advance_dialog(COLOR_RED);
        OverworldWatcher    overworld(COLOR_CYAN);
        PromptDialogWatcher prompt(COLOR_YELLOW);
        WhiteButtonWatcher  whitebutton(COLOR_GREEN, WhiteButton::ButtonA_DarkBackground, {0.725, 0.833, 0.024, 0.045}); // {0.650, 0.650, 0.140, 0.240}
        DialogArrowWatcher dialog_arrow(COLOR_RED, console.overlay(), {0.850, 0.820, 0.020, 0.050}, 0.8365, 0.846);
        NormalBattleMenuWatcher battle(COLOR_ORANGE);
        TutorialWatcher     tutorial(COLOR_BLUE);
        DialogBoxWatcher black_dialog_box(COLOR_BLACK, true, std::chrono::milliseconds(250), DialogType::DIALOG_BLACK);
        context.wait_for_all_requests();

        std::vector<PeriodicInferenceCallback> callbacks; 
        std::vector<ClearDialogCallback> enum_all_callbacks{ClearDialogCallback::ADVANCE_DIALOG}; // mandatory callbacks
        enum_all_callbacks.insert(enum_all_callbacks.end(), enum_optional_callbacks.begin(), enum_optional_callbacks.end()); // append the mandatory and optional callback vectors together
        for (const ClearDialogCallback& enum_callback : enum_all_callbacks){
            switch(enum_callback){
            case ClearDialogCallback::ADVANCE_DIALOG:
                callbacks.emplace_back(advance_dialog);
                break;                
            case ClearDialogCallback::OVERWORLD:
                callbacks.emplace_back(overworld);
                break;
            case ClearDialogCallback::PROMPT_DIALOG:
                callbacks.emplace_back(prompt);
                break;
            case ClearDialogCallback::WHITE_A_BUTTON:
                callbacks.emplace_back(whitebutton);
                break;
            case ClearDialogCallback::DIALOG_ARROW:
                callbacks.emplace_back(dialog_arrow);
                break;
            case ClearDialogCallback::BATTLE:
                callbacks.emplace_back(battle);
                break;
            case ClearDialogCallback::TUTORIAL:
                callbacks.emplace_back(tutorial);
                break;          
            case ClearDialogCallback::BLACK_DIALOG_BOX:
                callbacks.emplace_back(black_dialog_box);
                break;              
            }
        }


        int ret = wait_until(
            console, context,
            std::chrono::seconds(seconds_timeout),
            callbacks
        );
        // int ret = run_until(
        //     console, context,
        //     [&](BotBaseContext& context){
        //         for (size_t j = 0; j < seconds_timeout/3; j++){
        //             pbf_press_button(context, BUTTON_A, 20, 3*TICKS_PER_SECOND-20);
        //         }
        //     },
        //     {overworld, prompt, whitebutton, advance_dialog, battle}
        // );
        context.wait_for(std::chrono::milliseconds(100));
        if (ret < 0){
            console.log("clear_dialog(): Timed out.");
            if (seen_dialog && mode == ClearDialogMode::STOP_TIMEOUT){
                return;
            }
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "clear_dialog(): Timed out. Did not detect dialog or did not detect the expected stop condition.",
                true
            );
        }

        ClearDialogCallback enum_callback = enum_all_callbacks[ret];
        switch(enum_callback){
        case ClearDialogCallback::ADVANCE_DIALOG:
            console.log("clear_dialog: Detected advance dialog.");
            seen_dialog = true;
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;            
        case ClearDialogCallback::OVERWORLD:
            console.log("clear_dialog: Detected overworld.");
            if (seen_dialog && mode == ClearDialogMode::STOP_OVERWORLD){
                return;
            }
            break;
        case ClearDialogCallback::PROMPT_DIALOG:
            console.log("clear_dialog: Detected prompt.");
            seen_dialog = true;
            if (mode == ClearDialogMode::STOP_PROMPT){
                return;
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case ClearDialogCallback::WHITE_A_BUTTON:
            console.log("clear_dialog: Detected white A button.");
            seen_dialog = true;
            if (mode == ClearDialogMode::STOP_WHITEBUTTON){
                return;
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case ClearDialogCallback::DIALOG_ARROW:
            console.log("clear_dialog: Detected dialog arrow.");
            seen_dialog = true;
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case ClearDialogCallback::BATTLE:
            console.log("clear_dialog: Detected battle.");
            if (mode == ClearDialogMode::STOP_BATTLE){
                return;
            }
            break;
        case ClearDialogCallback::TUTORIAL:    
            console.log("clear_dialog: Detected tutorial.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case ClearDialogCallback::BLACK_DIALOG_BOX:    
            console.log("clear_dialog: Detected black dialog box.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;            
        default:
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "clear_dialog: Unknown callback triggered.");

        }        

    }
}

void overworld_navigation(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context,
    NavigationStopCondition stop_condition,
    NavigationMovementMode movement_mode,
    uint8_t x, uint8_t y,
    uint16_t seconds_timeout, uint16_t seconds_realign, 
    bool auto_heal
){
    bool should_realign = true;
    if (seconds_timeout <= seconds_realign){
        seconds_realign = seconds_timeout;
        should_realign = false;
    }
    uint16_t forward_ticks = seconds_realign * TICKS_PER_SECOND;
    // WallClock start = current_time();

    if (movement_mode == NavigationMovementMode::CLEAR_WITH_LETS_GO){
        context.wait_for(Milliseconds(3000)); // for some reason, the "Destination arrived" notification reappears when you re-assign the marker.
    }


    while (true){
        NormalBattleMenuWatcher battle(COLOR_BLUE);
        DialogBoxWatcher        dialog(COLOR_RED, true);
        DestinationMarkerWatcher marker(COLOR_CYAN, {0.717, 0.165, 0.03, 0.061});
        context.wait_for_all_requests();
        std::vector<PeriodicInferenceCallback> callbacks = {battle, dialog}; 
        if (stop_condition == NavigationStopCondition::STOP_MARKER){
            callbacks.emplace_back(marker);
        }
        // uint16_t ticks_passed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time() - start).count() * TICKS_PER_SECOND / 1000;
        // forward_ticks = seconds_realign * TICKS_PER_SECOND - ticks_passed;

        int ret = run_until(
            console, context,
            [&](BotBaseContext& context){

                for (int i = 0; i < seconds_timeout / seconds_realign; i++){
                    if (movement_mode == NavigationMovementMode::CLEAR_WITH_LETS_GO){
                        walk_forward_while_clear_front_path(info, console, context, forward_ticks, y);
                    }else{
                        ssf_press_left_joystick(context, x, y, 0, seconds_realign * TICKS_PER_SECOND);
                        if (movement_mode == NavigationMovementMode::DIRECTIONAL_ONLY){
                            pbf_wait(context, seconds_realign * TICKS_PER_SECOND);
                        } else if (movement_mode == NavigationMovementMode::DIRECTIONAL_SPAM_A){
                            for (size_t j = 0; j < seconds_realign; j++){
                                pbf_press_button(context, BUTTON_A, 20, 105);
                            }
                        }
                    }
                    if (should_realign){
                        realign_player(info, console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
                    }
                }
            },
            callbacks
        );
        context.wait_for(std::chrono::milliseconds(100));

        switch (ret){
        case 0: // battle
            console.log("overworld_navigation: Detected start of battle.");
            run_battle_press_A(console, context, BattleStopCondition::STOP_OVERWORLD);   
            if (auto_heal){
                auto_heal_from_menu_or_overworld(info, console, context, 0, true);
            }

            realign_player(info, console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            break;
        case 1: // dialog
            console.log("overworld_navigation: Detected dialog.");
            if (stop_condition == NavigationStopCondition::STOP_DIALOG){
                return;
            }
            if (stop_condition == NavigationStopCondition::STOP_MARKER){
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT, console,
                    "overworld_navigation(): Unexpectedly detected dialog.",
                    true
                );
            }            
            break;
        case 2: // marker
            console.log("overworld_navigation: Detected marker.");
            if (stop_condition == NavigationStopCondition::STOP_MARKER){
                return;
            }
            break;
        default:
            console.log("overworld_navigation(): Timed out.");
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "overworld_navigation(): Timed out. Did not detect expected stop condition.",
                true
            );
        }
    }
}

void config_option(BotBaseContext& context, int change_option_value){
    for (int i = 0; i < change_option_value; i++){
        pbf_press_dpad(context, DPAD_RIGHT, 15, 20);
    }
    pbf_press_dpad(context, DPAD_DOWN,  15, 20);
}

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
        context.wait_for_all_requests();
        VideoSnapshot screen = console.video().snapshot();
        PokemonMovesReader reader(language);
        std::string top_move = reader.read_move(console, screen, 0);
        console.log("Current top move: " + top_move);
        if (top_move != "ember" && top_move != "leafage" && top_move != "water-gun"){
            console.log("Failed to swap moves. Re-try.");
            continue;
        }   


        break;    
    }

}


void change_settings_prior_to_autostory(SingleSwitchProgramEnvironment& env, BotBaseContext& context, StartPoint current_segment, Language language){
    if (current_segment == StartPoint::INTRO_CUTSCENE){
        return;
    }

    // get index of `Options` in the Main Menu, depending on where you are in Autostory
    int8_t index;  
    switch(current_segment){
    // case StartPoint::INTRO_CUTSCENE:
    case StartPoint::PICK_STARTER:
        index = 0;
    case StartPoint::NEMONA_FIRST_BATTLE:
        index = 1;
    case StartPoint::CATCH_TUTORIAL:
    case StartPoint::LEGENDARY_RESCUE:
    case StartPoint::ARVEN_FIRST_BATTLE:
    case StartPoint::LOS_PLATOS:
    case StartPoint::MESAGOZA_SOUTH:
        index = 2;
    default:
        index = -1;        
    }
    
    if (index < 0){
        return;
    }
    bool has_minimap = current_segment != StartPoint::PICK_STARTER; // and also not equal to StartPoint::INTRO_CUTSCENE:

    enter_menu_from_overworld(env.program_info(), env.console, context, index, MenuSide::RIGHT, has_minimap);
    change_settings(env, context, language);
    if(current_segment == StartPoint::PICK_STARTER){
        pbf_mash_button(context, BUTTON_B, 2 * TICKS_PER_SECOND);
    }else{
        press_Bs_to_back_to_overworld(env.program_info(), env.console, context);    
    }
}

void change_settings(SingleSwitchProgramEnvironment& env, BotBaseContext& context,  Language language, bool use_inference){
    env.console.log("Update settings.");
    if (use_inference){
        MenuOption session(env.console, context, language);

        std::vector<std::pair<MenuOptionItemEnum, std::vector<MenuOptionToggleEnum>>> options = {
            {MenuOptionItemEnum::TEXT_SPEED, {MenuOptionToggleEnum::FAST}},
            {MenuOptionItemEnum::SKIP_MOVE_LEARNING, {MenuOptionToggleEnum::ON}},
            {MenuOptionItemEnum::SEND_TO_BOXES, {MenuOptionToggleEnum::AUTOMATIC, MenuOptionToggleEnum::ON}},
            {MenuOptionItemEnum::GIVE_NICKNAMES, {MenuOptionToggleEnum::OFF}},
            {MenuOptionItemEnum::VERTICAL_CAMERA_CONTROLS, {MenuOptionToggleEnum::REGULAR, MenuOptionToggleEnum::NORMAL}},
            {MenuOptionItemEnum::HORIZONTAL_CAMERA_CONTROLS, {MenuOptionToggleEnum::REGULAR, MenuOptionToggleEnum::NORMAL}},
            {MenuOptionItemEnum::CAMERA_SUPPORT, {MenuOptionToggleEnum::ON}},
            {MenuOptionItemEnum::CAMERA_INTERPOLATION, {MenuOptionToggleEnum::NORMAL, MenuOptionToggleEnum::AVERAGE}},
            {MenuOptionItemEnum::CAMERA_DISTANCE, {MenuOptionToggleEnum::CLOSE}},
            {MenuOptionItemEnum::AUTOSAVE, {MenuOptionToggleEnum::OFF}},
            {MenuOptionItemEnum::SHOW_NICKNAMES, {MenuOptionToggleEnum::OFF, MenuOptionToggleEnum::DONT_SHOW}},
            {MenuOptionItemEnum::SKIP_CUTSCENES, {MenuOptionToggleEnum::ON}},
            {MenuOptionItemEnum::CONTROLLER_RUMBLE, {MenuOptionToggleEnum::ON}},
            {MenuOptionItemEnum::HELPING_FUNCTIONS, {MenuOptionToggleEnum::OFF}},

        };
        session.set_options(options);        
    }else{
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
    clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 5, {ClearDialogCallback::PROMPT_DIALOG});
    
}

void do_action_and_monitor_for_battles(
    SingleSwitchProgramEnvironment& env,
    ConsoleHandle& console, 
    BotBaseContext& context,
    std::function<
        void(SingleSwitchProgramEnvironment& env,
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

void wait_for_gradient_arrow(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context, 
    ImageFloatBox box_area_to_check,
    uint16_t seconds_timeout
){
    context.wait_for_all_requests();
    GradientArrowWatcher arrow(COLOR_RED, GradientArrowType::RIGHT, box_area_to_check);
    int ret = wait_until(
        console, context, 
        Milliseconds(seconds_timeout * 1000),
        { arrow }
    );
    if (ret == 0){
        console.log("Gradient arrow detected.");
    }else{
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "Failed to detect gradient arrow.",
            true
        );
    }          
}

void press_A_until_dialog(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, uint16_t seconds_between_button_presses){
    context.wait_for_all_requests();
    AdvanceDialogWatcher advance_dialog(COLOR_RED);
    int ret = run_until(
        console, context,
        [seconds_between_button_presses](BotBaseContext& context){
            pbf_wait(context, seconds_between_button_presses * TICKS_PER_SECOND); // avoiding pressing A if dialog already present
            for (size_t c = 0; c < 10; c++){
                pbf_press_button(context, BUTTON_A, 20, seconds_between_button_presses * TICKS_PER_SECOND);
            }
        },
        {advance_dialog}
    );
    if (ret == 0){
        console.log("press_A_until_dialog: Detected dialog.");
    }else{
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "press_A_until_dialog(): Unable to detect dialog after 10 button presses.",
            true
        );
    }
}

bool check_ride_active(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    // open main menu
    enter_menu_from_overworld(info, console, context, -1, MenuSide::NONE, true);
    context.wait_for_all_requests();
    ImageStats ride_indicator = image_stats(extract_box_reference(console.video().snapshot(), ImageFloatBox(0.05, 0.995, 0.25, 0.003)));

    bool is_ride_active = !is_black(ride_indicator); // ride is active if the ride indicator isn't black.
    pbf_press_button(context, BUTTON_B, 30, 100);
    press_Bs_to_back_to_overworld(info, console, context, 7);
    if (is_ride_active){
        console.log("Ride is active.");
    }else{
        console.log("Ride is not active.");
    }
    return is_ride_active;
}

void get_on_ride(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    WallClock start = current_time();
    while (!check_ride_active(info, console, context)){
        if (current_time() - start > std::chrono::minutes(3)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "get_on_ride(): Failed to get on ride after 3 minutes.",
                true
            );
        }        
        pbf_press_button(context, BUTTON_PLUS, 30, 100);
    }
}

void checkpoint_save(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    save_game_from_overworld(env.program_info(), env.console, context);
    stats.m_checkpoint++;
    env.update_stats();
    send_program_status_notification(env, notif_status_update, "Saved at checkpoint.");        
}

void checkpoint_00(SingleSwitchProgramEnvironment& env, BotBaseContext& context){


    // Mash A through intro cutscene, until the L stick button is detected
    WhiteButtonWatcher leftstick(COLOR_GREEN, WhiteButton::ButtonLStick, {0.435, 0.912, 0.046, 0.047});
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

}

void checkpoint_01(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update, 
    Language language
){
AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if(first_attempt){
            save_game_tutorial(env.program_info(), env.console, context);
            stats.m_checkpoint++;
            env.update_stats();
            send_program_status_notification(env, notif_status_update, "Saved at checkpoint.");     
            first_attempt = false;
        }
        
        context.wait_for_all_requests();
        // set settings
        enter_menu_from_overworld(env.program_info(), env.console, context, 0, MenuSide::RIGHT, false);
        change_settings(env, context, language, first_attempt);
        pbf_mash_button(context, BUTTON_B, 2 * TICKS_PER_SECOND);

        break;  
    }catch(...){
        // (void)e;
        first_attempt = false;
        context.wait_for_all_requests();
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }
    }
}

void checkpoint_02(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if(!first_attempt){
            save_game_tutorial(env.program_info(), env.console, context);
            stats.m_checkpoint++;
            env.update_stats();
            send_program_status_notification(env, notif_status_update, "Saved at checkpoint.");     
            first_attempt = false;
        }
        
        context.wait_for_all_requests();
        env.console.log("Go downstairs, get stopped by Skwovet");
        env.console.overlay().add_log("Go downstairs, get stopped by Skwovet", COLOR_WHITE);
        pbf_move_left_joystick(context, 128,   0, 3 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context,   0, 128, 3 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context, 128, 255, 3 * TICKS_PER_SECOND, 20);
        pbf_wait(context, 5 * TICKS_PER_SECOND);
        // clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 5, {});

        context.wait_for_all_requests();
        env.console.log("Go to the kitchen, talk with mom");
        env.console.overlay().add_log("Go to the kitchen, talk with mom", COLOR_WHITE);
        pbf_move_left_joystick(context, 128, 255, 2 * TICKS_PER_SECOND, 20);
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 0, 128);

        env.console.log("clear_dialog: Talk with Mom.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 5, {});

        context.wait_for_all_requests();
        env.console.log("Go to the front door, talk with Clavell");
        env.console.overlay().add_log("Go to the front door, talk with Clavell", COLOR_WHITE);
        pbf_move_left_joystick(context, 230, 200, 2 * TICKS_PER_SECOND, 20);
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 255, 128);
        
        env.console.log("clear_dialog: Talk with Clavell at front door.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 5, {});

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
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 0, 128);
        
        env.console.log("clear_dialog: Talk with Clavell at living room.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10, {});

        context.wait_for_all_requests();
        env.console.log("Go outside, receive Rotom Phone");
        env.console.overlay().add_log("Go outside, receive Rotom Phone", COLOR_WHITE);
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 245, 230);

        env.console.log("clear_dialog: Talk with Clavell outside. Receive Rotom phone. Stop when detect overworld.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {ClearDialogCallback::OVERWORLD, ClearDialogCallback::WHITE_A_BUTTON});

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

void checkpoint_03(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update,
    Language language,
    StarterChoice starter_choice
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){   
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }
        
        context.wait_for_all_requests();
        
        pbf_move_left_joystick(context, 255, 0, 1 * TICKS_PER_SECOND, 20);
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 156, 1 * TICKS_PER_SECOND);
        env.console.log("overworld_navigation(): Go to Nemona's house.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0);
        
        context.wait_for_all_requests();
        env.console.log("Entered Nemona's house");
        mash_button_till_overworld(env.console, context);
        context.wait_for_all_requests();
        env.console.log("Picking a starter...");
        switch(starter_choice){
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
        env.console.log("clear_dialog: Choose starter. Stop when detect prompt to receive starter.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 20, {ClearDialogCallback::PROMPT_DIALOG});

        pbf_press_button(context, BUTTON_A, 20, 105); // accept the pokemon
        env.console.log("clear_dialog: Stop when detect prompt to give nickname to starter.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 20, {ClearDialogCallback::PROMPT_DIALOG});

        pbf_mash_button(context, BUTTON_B, 100);  // Don't give a nickname
        env.console.log("clear_dialog: Talk to Nemona and Clavell. Stop when detect overworld.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 20, {ClearDialogCallback::OVERWORLD});
        
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
        swap_starter_moves(env.program_info(), env.console, context, language);
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

void checkpoint_04(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{        
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }        
        context.wait_for_all_requests();

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 220, 245, 50);
        pbf_move_left_joystick(context, 128, 0, 4 * TICKS_PER_SECOND, 1 * TICKS_PER_SECOND);
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 128, 50);
        pbf_move_left_joystick(context, 128, 0, 4 * TICKS_PER_SECOND, 1 * TICKS_PER_SECOND);
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 60, 50);
        pbf_move_left_joystick(context, 128, 0, 4 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
        env.console.log("overworld_navigation: Go to Nemona at the beach.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_SPAM_A, 128, 0, 8);
        
        context.wait_for_all_requests();
        env.console.overlay().add_log("Found Nemona", COLOR_WHITE);

        context.wait_for_all_requests();
        env.console.log("Starting battle...");
        env.console.overlay().add_log("Starting battle...", COLOR_WHITE);
        // TODO: Battle start prompt detection
        // can lose this battle, and story will continue
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

void checkpoint_05(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 40, 160, 60);
        pbf_move_left_joystick(context, 128, 0, 7 * TICKS_PER_SECOND, 20);
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 40, 84, 60);
        env.console.log("overworld_navigation: Go to mom at the gate.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0, 20);
        
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

void checkpoint_06(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{        
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }        

        context.wait_for_all_requests();

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 40, 82, 60);
        pbf_move_left_joystick(context, 128, 0, 6 * TICKS_PER_SECOND, 20);
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 110, 10, 60);
        env.console.log("overworld_navigation: Go to Nemona.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0, 20);
        
        context.wait_for_all_requests();
        env.console.log("clear_dialog: Talk with Nemona to start catch tutorial. Stop when detect battle.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, 
            {ClearDialogCallback::WHITE_A_BUTTON, ClearDialogCallback::TUTORIAL, ClearDialogCallback::BATTLE});
        
        // can die in catch tutorial, and the story will continue
        env.console.log("run_battle_press_A: Battle Lechonk in catch tutorial. Stop when detect dialog.");
        run_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);

        env.console.log("clear_dialog: Talk with Nemona to finish catch tutorial. Stop when detect overworld.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, 
            {ClearDialogCallback::TUTORIAL, ClearDialogCallback::OVERWORLD});

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

void checkpoint_07(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         

        context.wait_for_all_requests();
        env.console.log("Move to cliff");
        env.console.overlay().add_log("Move to cliff", COLOR_WHITE);

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 240, 60, 80);
        env.console.log("overworld_navigation: Go to cliff.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 116, 0, 72, 24);

        env.console.log("clear_dialog: Talk to Nemona at the cliff. Stop when detect overworld.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {ClearDialogCallback::OVERWORLD});

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

void checkpoint_08(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 230, 70, 100);
        env.console.log("overworld_navigation: Go to cliff.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0);

        env.console.log("clear_dialog: Look over the injured Miraidon/Koraidon on the beach. Fall down the cliff");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 30, {});
        // long animation
        env.console.log("overworld_navigation: Go to Legendary pokemon laying on the beach.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0, 30);

        env.console.log("clear_dialog: Offer Miraidon/Koraidon a sandwich.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10, {});

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

        env.console.log("clear_dialog: Miraidon/Koraidon gets up and walks to cave entrance.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 35, {});
        // long animation

        // First Nemona cave conversation
        context.wait_for_all_requests();
        env.console.log("Enter cave");
        env.console.overlay().add_log("Enter cave", COLOR_WHITE);
        do_action_and_monitor_for_battles(env, env.console, context,
            [&](SingleSwitchProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
                pbf_move_left_joystick(context, 128, 20, 10 * TICKS_PER_SECOND, 20);
                pbf_move_left_joystick(context, 150, 20, 1 * TICKS_PER_SECOND, 20);
                pbf_move_left_joystick(context, 128, 20, 8 * TICKS_PER_SECOND, 20);
                pbf_move_left_joystick(context, 150, 20, 2 * TICKS_PER_SECOND, 20);                
            }
        );
        
        env.console.log("overworld_navigation: Go to cave.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 20, 10);

        env.console.log("clear_dialog: Talk to Nemona yelling down, while you're down in the cave.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10, {ClearDialogCallback::PROMPT_DIALOG});

        do_action_and_monitor_for_battles(env, env.console, context,
            [&](SingleSwitchProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
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
                pbf_move_left_joystick(context, 140, 20, 4 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NO_MARKER, 220, 15, 30);
                pbf_move_left_joystick(context, 128, 20, 5 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
                pbf_move_left_joystick(context, 128, 20, 6 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NO_MARKER, 220, 25, 20);
                pbf_move_left_joystick(context, 128, 20, 4 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
                pbf_move_left_joystick(context, 128, 20, 4 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
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
        
        env.console.log("overworld_navigation: Go to Houndoom.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 20, 40);
        
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


void checkpoint_09(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{        
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }      
        context.wait_for_all_requests();

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 230, 120, 100);
        env.console.log("overworld_navigation: Go to Arven at the tower.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0);
            
        context.wait_for_all_requests();
        env.console.log("Found Arven");
        env.console.overlay().add_log("Found Arven", COLOR_WHITE);
        // can lose battle, and story will still continue
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

void checkpoint_10(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }        
        context.wait_for_all_requests();
        env.console.log("Lighthouse view");
        env.console.overlay().add_log("Lighthouse view", COLOR_WHITE);
        do_action_and_monitor_for_battles(env, env.console, context,
            [&](SingleSwitchProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 230, 110, 100);
                pbf_move_left_joystick(context, 128, 0, 6 * TICKS_PER_SECOND, 8 * TICKS_PER_SECOND);
                pbf_move_left_joystick(context, 128, 0, 4 * TICKS_PER_SECOND, 20);
                pbf_move_left_joystick(context, 255, 128, 15, 20);
                pbf_press_button(context, BUTTON_L, 20, 20);
                pbf_move_left_joystick(context, 128, 0, 7 * TICKS_PER_SECOND, 20);                
            }
        );

        env.console.log("overworld_navigation: Go to Nemona on the lighthouse.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_SPAM_A, 128, 0, 20);

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

void checkpoint_11(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        } 

        context.wait_for_all_requests();
        do_action_and_monitor_for_battles(env, env.console, context,
            [&](SingleSwitchProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 100, 210, 100);
                pbf_move_left_joystick(context, 128, 0, 187, 20);
                pbf_move_left_joystick(context, 0, 128, 30, 8 * TICKS_PER_SECOND);
                pbf_move_left_joystick(context, 128, 0, 1 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);

                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 100, 60, 200);                
            }
        );     

        env.console.log("overworld_navigation: Go to Los Platos.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0, 75);

        env.console.log("clear_dialog: Talk with Nemona at Los Platos. Clear Let's go tutorial. Stop when detect overworld.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {ClearDialogCallback::TUTORIAL, ClearDialogCallback::OVERWORLD});

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


void checkpoint_12(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    // reset rate: ~25%. 12 resets out of 52. 
    // resets due to: getting attacked by wild pokemon, either from behind, 
    // or when lead pokemon not strong enough to clear them with Let's go
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        do_action_and_monitor_for_battles(env, env.console, context,
        [&](SingleSwitchProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){        
        
            if (first_attempt){
                checkpoint_save(env, context, notif_status_update);
                first_attempt = false;
            } 

            context.wait_for_all_requests();
            
            fly_to_overlapping_flypoint(env.program_info(), env.console, context);

            // re-orient camera
            pbf_press_button(context, BUTTON_L, 20, 20);

            walk_forward_while_clear_front_path(env.program_info(), env.console, context, 35);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 128, 0, 100);
            walk_forward_while_clear_front_path(env.program_info(), env.console, context, 500);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 100, 0, 120);
            walk_forward_while_clear_front_path(env.program_info(), env.console, context, 2000);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 0, 120);
            walk_forward_while_clear_front_path(env.program_info(), env.console, context, 1250);
            
            // check we're not still at the Los Platos Pokecenter.
            confirm_no_overlapping_flypoint(env.program_info(), env.console, context);

            // not stuck at Los Platos Pokecenter
            pbf_press_button(context, BUTTON_B, 20, 1 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_B, 20, 1 * TICKS_PER_SECOND);
            press_Bs_to_back_to_overworld(env.program_info(), env.console, context, 7);

            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 80, 0, 100);
            walk_forward_while_clear_front_path(env.program_info(), env.console, context, 1500);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 30, 0, 50);
            walk_forward_while_clear_front_path(env.program_info(), env.console, context, 1000);

            heal_at_pokecenter(env.program_info(), env.console, context);
   
        }  
        );

        env.console.log("Reached Mesagoza (South) Pokecenter.");
        
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

void checkpoint_13(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    // reset rate: 0%. 0 resets out of 70.
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        do_action_and_monitor_for_battles(env, env.console, context,
        [&](SingleSwitchProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){        
        
            if (first_attempt){
                checkpoint_save(env, context, notif_status_update);
                first_attempt = false;
            } 

            fly_to_overlapping_flypoint(env.program_info(), env.console, context);

            context.wait_for_all_requests();
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 80, 50);
            walk_forward_while_clear_front_path(env.program_info(), env.console, context, 500);
            walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 30);
        });

        env.console.log("clear_dialog: Talk with Nemona at Mesagoza gate. Stop when detect battle.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60,
            {ClearDialogCallback::PROMPT_DIALOG, ClearDialogCallback::DIALOG_ARROW, ClearDialogCallback::BATTLE});
        
        env.console.log("run_battle_press_A: Battle with Nemona at Mesagoza gate. Stop when detect dialog.");
        run_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        
        env.console.log("clear_dialog: Talk with Nemona within Mesagoza. Stop when detect overworld.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, 
            {ClearDialogCallback::OVERWORLD, ClearDialogCallback::PROMPT_DIALOG, ClearDialogCallback::WHITE_A_BUTTON});
        
       
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

void checkpoint_14(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();
        // realign diagonally to the left
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 80, 0, 100);
        // walk forward so you're off center
        pbf_move_left_joystick(context, 128, 0, 100, 100);
        // realign going straight
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 128, 0, 100);
        // walk forward, while still off center
        pbf_move_left_joystick(context, 128, 0, 2000, 100);
        // realign diagonally to the right
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 178, 0, 100);
        // walk forward so you're closer to the center
        pbf_move_left_joystick(context, 128, 0, 150, 100);
        // realign going straight
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 128, 0, 100);
        // walk forward until hit dialog at top of stairs
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 60);
        // clear dialog until battle. with prompt, battle
        env.console.log("clear_dialog: Talk with Team Star at the top of the stairs. Stop when detect battle.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {ClearDialogCallback::PROMPT_DIALOG, ClearDialogCallback::BATTLE, ClearDialogCallback::DIALOG_ARROW});
        // run battle until dialog
        env.console.log("run_battle_press_A: Battle with Team Star grunt 1. Stop when detect dialog.");
        run_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        // clear dialog until battle, with prompt, white button, tutorial, battle
        env.console.log("clear_dialog: Talk with Team Star and Nemona. Receive Tera orb. Stop when detect battle.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, 
            {ClearDialogCallback::PROMPT_DIALOG, ClearDialogCallback::WHITE_A_BUTTON, ClearDialogCallback::TUTORIAL, ClearDialogCallback::BATTLE, ClearDialogCallback::DIALOG_ARROW});
        // run battle until dialog
        env.console.log("run_battle_press_A: Battle with Team Star grunt 2. Stop when detect dialog.");
        run_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        // clear dialog until overworld
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {ClearDialogCallback::OVERWORLD});
       
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

void checkpoint_15(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();
        // realign diagonally to the right
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 178, 0, 100);
        // walk forward so you're closer to the center
        pbf_move_left_joystick(context, 128, 0, 100, 100);
        // realign going straight
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 128, 0, 100);
        // walk forward up stairs
        pbf_move_left_joystick(context, 128, 0, 1000, 100);
        // realign going straight
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        // walk forward until hit dialog inside the school
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 60);

        env.console.log("clear_dialog: Talk with Nemona, Clavell, and Jacq inside the school. Stop when detect overworld.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, 
            {ClearDialogCallback::PROMPT_DIALOG, ClearDialogCallback::OVERWORLD});
       
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

void checkpoint_16(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();

        // walk left
        pbf_move_left_joystick(context, 0, 128, 400, 100);
        // walk down to classroom exit.
        pbf_move_left_joystick(context, 128, 255, 300, 100);
        env.console.log("clear_dialog: Leave classroom.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 5);

        // Wait for detection of school navigation menu
        wait_for_gradient_arrow(env.program_info(), env.console, context, {0.031, 0.193, 0.047, 0.078}, 5);

        // enter Cafeteria
        pbf_mash_button(context, BUTTON_A, 3 * TICKS_PER_SECOND);
        pbf_wait(context, 3 * TICKS_PER_SECOND);
        context.wait_for_all_requests();

        // walk forward
        pbf_move_left_joystick(context, 128, 0, 300, 100);
        // turn left 
        pbf_move_left_joystick(context, 0, 128, 50, 100);

        // talk to Arven. stop at overworld. need prompt, overworld, white button A. and book?
        env.console.log("Talk with Arven. Receive Titan questline (Path of Legends).");
        press_A_until_dialog(env.program_info(), env.console, context, 1);
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);
        
       
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

void checkpoint_17(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();

        // walk backwards until dialog
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 20, 255);
        env.console.log("Talk with Cassiopeia.");
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

        // re-orient camera
        pbf_press_button(context, BUTTON_L, 20, 100);
        // move backwards towards front desk
        pbf_move_left_joystick(context, 128, 255, 200, 100);
        // re-orient camera
        pbf_press_button(context, BUTTON_L, 20, 100);
        // move right towards navigation kiosk
        pbf_move_left_joystick(context, 255, 128, 100, 100);
        // open school navigation screen
        press_button_until_gradient_arrow(env.program_info(), env.console, context, {0.031, 0.193, 0.047, 0.078});
        // go to staff room
        basic_menu_navigation(env.program_info(), env.console, context, {0.031, 0.193, 0.047, 0.078}, {0.031, 0.193 + 0.074219, 0.047, 0.078}, DPAD_DOWN, 1);
        // enter staff room
        pbf_mash_button(context, BUTTON_A, 3 * TICKS_PER_SECOND);
        pbf_wait(context, 3 * TICKS_PER_SECOND);

        env.console.log("clear_dialog: See Geeta. Talk to Nemona. Receive Gym/Elite Four questline (Victory Road).");
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, 
            {ClearDialogCallback::OVERWORLD, ClearDialogCallback::PROMPT_DIALOG});

        
       
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

void checkpoint_18(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();

        // walk down
        pbf_move_left_joystick(context, 128, 255, 200, 100);
        // walk left towards door
        pbf_move_left_joystick(context, 0, 128, 100, 100);

        // wait for school navigation menu
        context.wait_for_all_requests();
        wait_for_gradient_arrow(env.program_info(), env.console, context, {0.031, 0.193, 0.047, 0.078}, 10);
        // enter Directors office
        pbf_mash_button(context, BUTTON_A, 6 * TICKS_PER_SECOND);

        env.console.log("Talk to Clavell in his office, and the professor.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 25, 
            {ClearDialogCallback::PROMPT_DIALOG});  // max time between dialog: 17s. set timeout to 25 seconds for buffer.
        // mash A to get through the Random A press that you need. when the professor shows you area zero.
        pbf_mash_button(context, BUTTON_A, 3 * TICKS_PER_SECOND);
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, 
            {ClearDialogCallback::OVERWORLD, ClearDialogCallback::PROMPT_DIALOG});
       
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

void checkpoint_19(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();

        // walk right 
        pbf_move_left_joystick(context, 255, 128, 50, 100);
        // walk down towards door
        pbf_move_left_joystick(context, 128, 255, 200, 100);

        env.console.log("Talk to Nemona and go to dorm.");
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

        // walk forward
        pbf_move_left_joystick(context, 128, 0, 100, 100);
        // walk left towards bed
        pbf_move_left_joystick(context, 0, 128, 100, 100);

        env.console.log("Go to bed. Time passes until treasure hunt.");
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

       
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

void checkpoint_20(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();
       
        //walk right towards door
        pbf_move_left_joystick(context, 255, 128, 200, 100);

        wait_for_gradient_arrow(env.program_info(), env.console, context, {0.031, 0.193, 0.047, 0.078}, 10);

        env.console.log("Leave dorm for schoolyard.");
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 60, 0);

        env.console.log("Talk to Nemona, Arven, Cassiopeia.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 16, 
            {ClearDialogCallback::PROMPT_DIALOG, ClearDialogCallback::BLACK_DIALOG_BOX}); // max time between dialog: 11
        
        // mash A to get through the Random A press that you need. when the Nemona shows you a Poke Gym.
        pbf_mash_button(context, BUTTON_A, 250);

        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10, 
            {ClearDialogCallback::TUTORIAL}); // max time between dialog: 3
        
        env.console.log("Get on ride.");
        pbf_mash_button(context, BUTTON_PLUS, 1 * TICKS_PER_SECOND);

        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

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

void checkpoint_21(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();
       
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

void checkpoint_22(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();
       
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

void checkpoint_23(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();
       
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

void checkpoint_24(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();
       
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

void checkpoint_25(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();
       
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

// todo: uncomment checkpoint_save and fly_to_overlapping_flypoint
void checkpoint_26(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            // checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();

        // fly_to_overlapping_flypoint(env.program_info(), env.console, context);

        // align for long stretch 1, part 1
        do_action_and_monitor_for_battles(env, env.console, context,
        [&](SingleSwitchProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){     
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 80, 0, 70);
        });

        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::CLEAR_WITH_LETS_GO, 
            128, 0, 60, 10, false);

        // align for long stretch 1, part 2
        do_action_and_monitor_for_battles(env, env.console, context,
        [&](SingleSwitchProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){     
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 120, 0, 55);
        });
        
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::CLEAR_WITH_LETS_GO, 
            128, 0, 60, 10, false);

        // align for long stretch 2
        do_action_and_monitor_for_battles(env, env.console, context,
        [&](SingleSwitchProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){     
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 70, 255, 100);

        });

        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::CLEAR_WITH_LETS_GO, 
            128, 0, 60, 10, false);

        // align for long stretch 3, part 1
        do_action_and_monitor_for_battles(env, env.console, context,
        [&](SingleSwitchProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){     
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 120, 0, 75);

        });
        
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::CLEAR_WITH_LETS_GO, 
            128, 0, 60, 10, false);

        // align for long stretch 3, part 2. // todo: consider realign based on pokecenter.
        do_action_and_monitor_for_battles(env, env.console, context,
        [&](SingleSwitchProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){     
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 35, 0, 85);

        });

        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::CLEAR_WITH_LETS_GO, 
            128, 0, 60, 10, false);        

        // align to cross bridge
        do_action_and_monitor_for_battles(env, env.console, context,
        [&](SingleSwitchProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){     
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 105, 30);

        });

        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::CLEAR_WITH_LETS_GO, 
            128, 0, 30, 30, false);    

                       
              
       
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

void checkpoint_27(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();
       
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

void checkpoint_28(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();
       
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

void checkpoint_29(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();
       
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

void checkpoint_30(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();
       
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

void checkpoint_31(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();
       
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

void checkpoint_32(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();
       
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

void checkpoint_33(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();
       
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

void checkpoint_34(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();
       
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




}
}
}
