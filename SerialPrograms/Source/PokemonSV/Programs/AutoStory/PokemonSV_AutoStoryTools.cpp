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
        OverworldWatcher        overworld(console, COLOR_CYAN);
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
    std::vector<CallbackEnum> enum_optional_callbacks
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
        OverworldWatcher    overworld(console, COLOR_CYAN);
        PromptDialogWatcher prompt(COLOR_YELLOW);
        WhiteButtonWatcher  whitebutton(COLOR_GREEN, WhiteButton::ButtonA_DarkBackground, {0.725, 0.833, 0.024, 0.045}); // {0.650, 0.650, 0.140, 0.240}
        DialogArrowWatcher dialog_arrow(COLOR_RED, console.overlay(), {0.850, 0.820, 0.020, 0.050}, 0.8365, 0.846);
        NormalBattleMenuWatcher battle(COLOR_ORANGE);
        TutorialWatcher     tutorial(COLOR_BLUE);
        DialogBoxWatcher black_dialog_box(COLOR_BLACK, true, std::chrono::milliseconds(250), DialogType::DIALOG_BLACK);
        context.wait_for_all_requests();

        std::vector<PeriodicInferenceCallback> callbacks; 
        std::vector<CallbackEnum> enum_all_callbacks{CallbackEnum::ADVANCE_DIALOG}; // mandatory callbacks
        enum_all_callbacks.insert(enum_all_callbacks.end(), enum_optional_callbacks.begin(), enum_optional_callbacks.end()); // append the mandatory and optional callback vectors together
        for (const CallbackEnum& enum_callback : enum_all_callbacks){
            switch(enum_callback){
            case CallbackEnum::ADVANCE_DIALOG:
                callbacks.emplace_back(advance_dialog);
                break;                
            case CallbackEnum::OVERWORLD:
                callbacks.emplace_back(overworld);
                break;
            case CallbackEnum::PROMPT_DIALOG:
                callbacks.emplace_back(prompt);
                break;
            case CallbackEnum::WHITE_A_BUTTON:
                callbacks.emplace_back(whitebutton);
                break;
            case CallbackEnum::DIALOG_ARROW:
                callbacks.emplace_back(dialog_arrow);
                break;
            case CallbackEnum::BATTLE:
                callbacks.emplace_back(battle);
                break;
            case CallbackEnum::TUTORIAL:
                callbacks.emplace_back(tutorial);
                break;          
            case CallbackEnum::BLACK_DIALOG_BOX:
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

        CallbackEnum enum_callback = enum_all_callbacks[ret];
        switch(enum_callback){
        case CallbackEnum::ADVANCE_DIALOG:
            console.log("clear_dialog: Detected advance dialog.");
            seen_dialog = true;
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;            
        case CallbackEnum::OVERWORLD:
            console.log("clear_dialog: Detected overworld.");
            if (seen_dialog && mode == ClearDialogMode::STOP_OVERWORLD){
                return;
            }
            break;
        case CallbackEnum::PROMPT_DIALOG:
            console.log("clear_dialog: Detected prompt.");
            seen_dialog = true;
            if (mode == ClearDialogMode::STOP_PROMPT){
                return;
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case CallbackEnum::WHITE_A_BUTTON:
            console.log("clear_dialog: Detected white A button.");
            seen_dialog = true;
            if (mode == ClearDialogMode::STOP_WHITEBUTTON){
                return;
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case CallbackEnum::DIALOG_ARROW:
            console.log("clear_dialog: Detected dialog arrow.");
            seen_dialog = true;
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case CallbackEnum::BATTLE:
            console.log("clear_dialog: Detected battle.");
            if (mode == ClearDialogMode::STOP_BATTLE){
                return;
            }
            break;
        case CallbackEnum::TUTORIAL:    
            console.log("clear_dialog: Detected tutorial.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case CallbackEnum::BLACK_DIALOG_BOX:    
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


void change_settings_prior_to_autostory(SingleSwitchProgramEnvironment& env, BotBaseContext& context, size_t current_segment_num, Language language){
    if (current_segment_num == 0){  // can't change settings in the intro cutscene
        return;
    }

    // get index of `Options` in the Main Menu, which depends on where you are in Autostory
    int8_t options_index;  
    switch(current_segment_num){
        case 0:
            options_index = 0;
            break;
        case 1:
            options_index = 1;
            break;
        default:
            options_index = 2;        
            break;
    }
    
    bool has_minimap = current_segment_num > 1;  // the minimap only shows up in segment 2 and beyond

    enter_menu_from_overworld(env.program_info(), env.console, context, options_index, MenuSide::RIGHT, has_minimap);
    change_settings(env, context, language);
    if(has_minimap){
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
    clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 5, {CallbackEnum::PROMPT_DIALOG});
    
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






}
}
}
