/*  AutoStoryTools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/UnexpectedBattleException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/OCR/OCR_NumberReader.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Programs/NintendoSwitch_SnapshotDumper.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_StationaryOverworldWatcher.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/Map/PokemonSV_MapMenuDetector.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/Battles/PokemonSV_Battles.h"
#include "PokemonSV/Programs/AutoStory/PokemonSV_MenuOption.h"
#include "PokemonSV/Inference/PokemonSV_TutorialDetector.h"
#include "PokemonSV/Inference/PokemonSV_PokemonMovesReader.h"
#include "PokemonSV/Inference/Map/PokemonSV_DestinationMarkerDetector.h"
#include "PokemonSV/Inference/Map/PokemonSV_MapPokeCenterIconDetector.h"
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
    BattleStopCondition stop_condition,
    std::vector<CallbackEnum> enum_optional_callbacks,
    bool detect_wipeout
){
    int16_t num_times_seen_overworld = 0;
    size_t consecutive_move_select = 0;
    while (true){
        NormalBattleMenuWatcher battle(COLOR_BLUE);
        SwapMenuWatcher         fainted(COLOR_PURPLE);
        OverworldWatcher        overworld(console, COLOR_CYAN);
        AdvanceDialogWatcher    dialog(COLOR_RED);
        DialogArrowWatcher dialog_arrow(COLOR_RED, console.overlay(), {0.850, 0.820, 0.020, 0.050}, 0.8365, 0.846);
        GradientArrowWatcher next_pokemon(COLOR_BLUE, GradientArrowType::RIGHT, {0.50, 0.51, 0.30, 0.10});
        MoveSelectWatcher move_select_menu(COLOR_YELLOW);

        std::vector<PeriodicInferenceCallback> callbacks; 
        //  mandatory callbacks: Battle, Overworld, Advance Dialog, Swap menu, Move select
        std::vector<CallbackEnum> enum_all_callbacks{CallbackEnum::BATTLE, CallbackEnum::OVERWORLD, CallbackEnum::ADVANCE_DIALOG, CallbackEnum::SWAP_MENU, CallbackEnum::MOVE_SELECT}; // mandatory callbacks
        enum_all_callbacks.insert(enum_all_callbacks.end(), enum_optional_callbacks.begin(), enum_optional_callbacks.end()); // append the mandatory and optional callback vectors together
        for (const CallbackEnum& enum_callback : enum_all_callbacks){
            switch(enum_callback){
            case CallbackEnum::ADVANCE_DIALOG:
                callbacks.emplace_back(dialog);
                break;                
            case CallbackEnum::OVERWORLD:
                callbacks.emplace_back(overworld);
                break;
            case CallbackEnum::DIALOG_ARROW:
                callbacks.emplace_back(dialog_arrow);
                break;
            case CallbackEnum::BATTLE:
                callbacks.emplace_back(battle);
                break;
            case CallbackEnum::GRADIENT_ARROW:
                callbacks.emplace_back(next_pokemon);
                break;
            case CallbackEnum::SWAP_MENU:  
                callbacks.emplace_back(fainted);
                break;                     
            case CallbackEnum::MOVE_SELECT:
                callbacks.emplace_back(move_select_menu);
                break;
            default:
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "run_battle_press_A: Unknown callback requested.");
            }
        }        
        context.wait_for_all_requests();

        int ret = wait_until(
            console, context,
            std::chrono::seconds(90),
            callbacks
        );
        context.wait_for(std::chrono::milliseconds(100));
        if (ret < 0){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "run_battle_press_A(): Timed out. Did not detect expected stop condition.",
                true
            );
        }        

        CallbackEnum enum_callback = enum_all_callbacks[ret];
        switch (enum_callback){
        case CallbackEnum::BATTLE: // battle
            console.log("Detected battle menu.");
            consecutive_move_select = 0;
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case CallbackEnum::MOVE_SELECT:
            console.log("Detected move select. Spam first move");
            consecutive_move_select++;
            select_top_move(console, context, consecutive_move_select);
            break;
        case CallbackEnum::OVERWORLD: // overworld
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
        case CallbackEnum::ADVANCE_DIALOG: // advance dialog
            console.log("Detected dialog.");

            if (detect_wipeout){
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
        case CallbackEnum::DIALOG_ARROW:  // dialog arrow
            console.log("run_battle_press_A: Detected dialog arrow.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case CallbackEnum::GRADIENT_ARROW:
            console.log("run_battle_press_A: Detected prompt for bringing in next pokemon. Keep current pokemon.");
            pbf_mash_button(context, BUTTON_B, 100);
            break;
        case CallbackEnum::SWAP_MENU:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "run_battle_press_A(): Lead pokemon fainted.",
                true
            );        
        default:
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "run_battle_press_A: Unknown callback triggered.");
          
        }
    }
}

void select_top_move(ConsoleHandle& console, BotBaseContext& context, size_t consecutive_move_select){
    if (consecutive_move_select > 3){
        // to handle case where move is disabled/out of PP/taunted
        console.log("Failed to select a move 3 times. Choosing a different move.", COLOR_RED);
        pbf_press_dpad(context, DPAD_DOWN, 20, 40);
    }
    pbf_mash_button(context, BUTTON_A, 100);

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
            default:
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "clear_dialog: Unknown callback requested.");                
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
            seen_dialog = true;
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;            
        default:
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "clear_dialog: Unknown callback triggered.");

        }        

    }
}

bool confirm_marker_present(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context
){
    while (true){
        DestinationMarkerWatcher marker(COLOR_RED, {0.815, 0.645, 0.180, 0.320}, true);
        NormalBattleMenuWatcher battle(COLOR_BLUE);

        int ret = wait_until(
            console, context, 
            std::chrono::seconds(5),
            {marker, battle}
        );
        switch (ret){
        case 0: // marker
            console.log("Confirmed that marker is still present.");
            return true;
        case 1: // battle
            throw UnexpectedBattleException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "confirm_marker_present(): Unexpectedly detected battle.",
                false
            );
        default:
            console.log("Destination marker not detected.");
            return false;
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
    bool auto_heal,
    bool detect_wipeout
){
    bool should_realign = true;
    if (seconds_timeout <= seconds_realign){
        seconds_realign = seconds_timeout;
        should_realign = false;
    }
    uint16_t forward_ticks = seconds_realign * TICKS_PER_SECOND;
    // WallClock start = current_time();

    if (stop_condition == NavigationStopCondition::STOP_MARKER){
        context.wait_for(Milliseconds(2000)); // the "Destination arrived" notification can sometimes reappear if you opened the map too quickly after you reached the previous marker.
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
                    context.wait_for_all_requests();
                    if (should_realign){
                        try {
                            realign_player(info, console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
                   
                        }catch (UnexpectedBattleException&){
                            pbf_wait(context, 30 * TICKS_PER_SECOND);  // catch exception to allow the battle callback to take over.
                        }
                        
                    }
                }
            },
            callbacks
        );
        context.wait_for(std::chrono::milliseconds(100));

        switch (ret){
        case 0: // battle
            console.log("overworld_navigation: Detected start of battle.");
            if (stop_condition == NavigationStopCondition::STOP_BATTLE){
                return;
            }

            run_battle_press_A(console, context, BattleStopCondition::STOP_OVERWORLD, {}, detect_wipeout);   
            if (auto_heal){
                auto_heal_from_menu_or_overworld(info, console, context, 0, true);
            }
            context.wait_for_all_requests();
            try {
                realign_player(info, console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
                if (stop_condition == NavigationStopCondition::STOP_MARKER && !confirm_marker_present(info, console, context)){  
                    // if marker not present when using marker based navigation, don't keep walking forward.
                    return;
                }

                break;
            }catch (UnexpectedBattleException&){
                break;
            }
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
            pbf_press_button(context, BUTTON_A, 20, 20);
            break;
        case 2: // marker
            console.log("overworld_navigation: Detected marker.");
            if (stop_condition == NavigationStopCondition::STOP_MARKER){
                return;
            }
            break;
        default:
            console.log("overworld_navigation(): Timed out.");
            if (stop_condition == NavigationStopCondition::STOP_TIME){
                return;
            }            
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
            {MenuOptionItemEnum::CAMERA_SUPPORT, {MenuOptionToggleEnum::OFF}},
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
        config_option(context, 1); // Camera Support: Off
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
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context,
    std::function<
        void(const ProgramInfo& info, 
        ConsoleHandle& console,
        BotBaseContext& context)
    >&& action
){
    NormalBattleMenuWatcher battle_menu(COLOR_RED);
    int ret = run_until(
        console, context,
        [&](BotBaseContext& context){
            context.wait_for_all_requests();
            action(info, console, context);
        },
        {battle_menu}
    );
    if (ret == 0){ // battle detected
        throw UnexpectedBattleException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "do_action_and_monitor_for_battles(): Detected battle. Failed to complete action.",
            true
        );

    }
}


void handle_unexpected_battles(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context,
    std::function<
        void(const ProgramInfo& info, 
        ConsoleHandle& console,
        BotBaseContext& context)
    >&& action
){
    while (true){
        try {
            context.wait_for_all_requests();
            action(info, console, context);
            return;
        }catch (UnexpectedBattleException&){
            run_battle_press_A(console, context, BattleStopCondition::STOP_OVERWORLD);
        }
    }
}

void handle_when_stationary_in_overworld(
    const ProgramInfo& info, 
    ConsoleHandle& console,
    BotBaseContext& context,
    std::function<
        void(const ProgramInfo& info, 
        ConsoleHandle& console,
        BotBaseContext& context)
    >&& action,
    std::function<
        void(const ProgramInfo& info, 
        ConsoleHandle& console,
        BotBaseContext& context)
    >&& recovery_action,
    size_t seconds_stationary,
    uint16_t minutes_timeout, 
    size_t max_failures
){
    
    WallClock start = current_time();
    size_t num_failures = 0;
    while (true){
        if (current_time() - start > std::chrono::minutes(minutes_timeout)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "handle_when_stationary_in_overworld(): Failed to complete action after " + std::to_string(minutes_timeout) + " minutes.",
                true
            );
        }
        StationaryOverworldWatcher stationary_overworld(COLOR_RED, {0.865, 0.825, 0.08, 0.1}, seconds_stationary);

        int ret = run_until(
            console, context,
            [&](BotBaseContext& context){
                context.wait_for_all_requests();
                action(info, console, context);
            },
            {stationary_overworld}        
        );
        if (ret < 0){
            // successfully completed action without being stuck in a position where the overworld is stationary.
            return;
        }else if (ret == 0){
            // if stationary in overworld, run recovery action then try action again
            console.log("Detected stationary overworld.");
            num_failures++;
            if (num_failures > max_failures){
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT, console,
                    "handle_when_stationary_in_overworld(): Failed to complete action within " + std::to_string(max_failures) + " attempts.",
                    true
                );                
            }
            context.wait_for_all_requests();
            recovery_action(info, console, context);    
        }
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

void wait_for_overworld(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context, 
    uint16_t seconds_timeout
){
    context.wait_for_all_requests();
    OverworldWatcher        overworld(console, COLOR_CYAN);
    int ret = wait_until(
        console, context, 
        Milliseconds(seconds_timeout * 1000),
        { overworld }
    );
    if (ret == 0){
        console.log("Overworld detected.");
    }else{
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "Failed to detect overworld.",
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

bool is_ride_active(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    while (true){
        try {
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

        }catch(UnexpectedBattleException&){
            run_battle_press_A(console, context, BattleStopCondition::STOP_OVERWORLD);
        }
    }

}

void get_on_ride(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    get_on_or_off_ride(info, console, context, true);
}

void get_off_ride(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    get_on_or_off_ride(info, console, context, false);
}

void get_on_or_off_ride(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, bool get_on){
    pbf_press_button(context, BUTTON_PLUS, 20, 20);

    WallClock start = current_time();
    while (get_on != is_ride_active(info, console, context)){
        if (current_time() - start > std::chrono::minutes(3)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "get_on_or_off_ride(): Failed to get on/off ride after 3 minutes.",
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


void realign_player_from_landmark(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context,
    MoveCursor move_cursor_near_landmark,
    MoveCursor move_cursor_to_target
){

    console.log("Realigning player direction, using a landmark...");
    WallClock start = current_time();

    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "realign_player_from_landmark(): Failed to realign player after 5 minutes.",
                true
            );
        }

        try {
            open_map_from_overworld(info, console, context, false);

            // move cursor near landmark (pokecenter)
            switch(move_cursor_near_landmark.zoom_change){
            case ZoomChange::ZOOM_IN:
                pbf_press_button(context, BUTTON_ZR, 20, 105);
                break;
            case ZoomChange::ZOOM_IN_TWICE:
                pbf_press_button(context, BUTTON_ZR, 20, 105);
                pbf_press_button(context, BUTTON_ZR, 20, 105);
                break;                
            case ZoomChange::ZOOM_OUT:
                pbf_press_button(context, BUTTON_ZL, 20, 105);
                break;    
            case ZoomChange::ZOOM_OUT_TWICE:
                pbf_press_button(context, BUTTON_ZL, 20, 105);
                pbf_press_button(context, BUTTON_ZL, 20, 105);
                break;                  
            case ZoomChange::KEEP_ZOOM:
                break;
            }
            uint8_t move_x1 = move_cursor_near_landmark.move_x;
            uint8_t move_y1 = move_cursor_near_landmark.move_y;
            uint16_t move_duration1 = move_cursor_near_landmark.move_duration;
            pbf_move_left_joystick(context, move_x1, move_y1, move_duration1, 1 * TICKS_PER_SECOND);

            // move cursor to pokecenter
            if (!detect_closest_pokecenter_and_move_map_cursor_there(info, console, context, 0.29)){
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT, console,
                    "realign_player_from_landmark(): No visible pokecenter found on map.",
                    true
                );         
            }

            confirm_cursor_centered_on_pokecenter(info, console, context); // throws exception if fails

            // move cursor from landmark to target
            switch(move_cursor_to_target.zoom_change){
            case ZoomChange::ZOOM_IN:
                pbf_press_button(context, BUTTON_ZR, 20, 105);
                break;
            case ZoomChange::ZOOM_IN_TWICE:
                pbf_press_button(context, BUTTON_ZR, 20, 105);
                pbf_press_button(context, BUTTON_ZR, 20, 105);
                break;                
            case ZoomChange::ZOOM_OUT:
                pbf_press_button(context, BUTTON_ZL, 20, 105);
                break;    
            case ZoomChange::ZOOM_OUT_TWICE:
                pbf_press_button(context, BUTTON_ZL, 20, 105);
                pbf_press_button(context, BUTTON_ZL, 20, 105);
                break;                  
            case ZoomChange::KEEP_ZOOM:
                break;
            }
            uint8_t move_x2 = move_cursor_to_target.move_x;
            uint8_t move_y2 = move_cursor_to_target.move_y;
            uint16_t move_duration2 = move_cursor_to_target.move_duration;
            pbf_move_left_joystick(context, move_x2, move_y2, move_duration2, 1 * TICKS_PER_SECOND);

            // place down marker
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_button(context, BUTTON_A, 20, 105);
            leave_phone_to_overworld(info, console, context);

            return;      

        }catch (OperationFailedException&){
            // reset to overworld if failed to center on the pokecenter, and re-try
            leave_phone_to_overworld(info, console, context);
        }catch (UnexpectedBattleException&){
            run_battle_press_A(console, context, BattleStopCondition::STOP_OVERWORLD);
        }
    }
    

}


void confirm_cursor_centered_on_pokecenter(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    context.wait_for_all_requests();
    context.wait_for(Milliseconds(500));
    ImageFloatBox center_cursor{0.484, 0.472, 0.030, 0.053};
    MapPokeCenterIconDetector pokecenter(COLOR_RED, center_cursor);
    if (!pokecenter.detect(console.video().snapshot())){
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "confirm_cursor_centered_on_pokecenter(): Cursor is not centered on a pokecenter.",
            true
        );            
    }

    console.log("Confirmed that the cursor is centered on a pokecenter.");
}

void move_cursor_towards_flypoint_and_go_there(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context,
    MoveCursor move_cursor_near_flypoint
){
    WallClock start = current_time();

    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "move_cursor_towards_flypoint_and_go_there(): Failed to fly after 5 minutes.",
                true
            );
        }

        try {
            open_map_from_overworld(info, console, context, false);

            // move cursor near landmark (pokecenter)
            switch(move_cursor_near_flypoint.zoom_change){
            case ZoomChange::ZOOM_IN:
                pbf_press_button(context, BUTTON_ZR, 20, 105);
                break;
            case ZoomChange::ZOOM_IN_TWICE:
                pbf_press_button(context, BUTTON_ZR, 20, 105);
                pbf_press_button(context, BUTTON_ZR, 20, 105);
                break;                
            case ZoomChange::ZOOM_OUT:
                pbf_press_button(context, BUTTON_ZL, 20, 105);
                break;    
            case ZoomChange::ZOOM_OUT_TWICE:
                pbf_press_button(context, BUTTON_ZL, 20, 105);
                pbf_press_button(context, BUTTON_ZL, 20, 105);
                break;                  
            case ZoomChange::KEEP_ZOOM:
                break;
            }
            uint8_t move_x1 = move_cursor_near_flypoint.move_x;
            uint8_t move_y1 = move_cursor_near_flypoint.move_y;
            uint16_t move_duration1 = move_cursor_near_flypoint.move_duration;
            pbf_move_left_joystick(context, move_x1, move_y1, move_duration1, 1 * TICKS_PER_SECOND);

            if (!fly_to_visible_closest_pokecenter_cur_zoom_level(info, console, context)){
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT, console,
                    "move_cursor_towards_flypoint_and_go_there(): No visible pokecenter found on map.",
                    true
                );                  
            }

            return;      

        }catch (OperationFailedException&){
            // reset to overworld if failed to center on the pokecenter, and re-try
            leave_phone_to_overworld(info, console, context);
        }catch (UnexpectedBattleException&){
            run_battle_press_A(console, context, BattleStopCondition::STOP_OVERWORLD);
        }
    }
    

}



void check_num_sunflora_found(SingleSwitchProgramEnvironment& env, BotBaseContext& context, int expected_number){
    context.wait_for_all_requests();
    VideoSnapshot screen = env.console.video().snapshot();
    ImageFloatBox num_sunflora_box = {0.27, 0.02, 0.04, 0.055};
    int number = OCR::read_number_waterfill(env.console, extract_box_reference(screen, num_sunflora_box), 0xff000000, 0xff808080);

    if (number != expected_number){
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT,
            env.logger(),
            "The number of sunflora found is different than expected."
        );
    }else{
        env.console.log("Number of sunflora found: " + std::to_string(number));
    }


}



}
}
}
