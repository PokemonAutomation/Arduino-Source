/*  AutoStoryTools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/UnexpectedBattleException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_StationaryOverworldWatcher.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
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

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{






void clear_tutorial(VideoStream& stream, ProControllerContext& context, uint16_t seconds_timeout){
    bool seen_tutorial = false;
    while (true){
        TutorialWatcher tutorial;
        context.wait_for_all_requests();

        int ret = wait_until(
            stream, context,
            std::chrono::seconds(seconds_timeout),
            {tutorial}
        );
        context.wait_for(std::chrono::milliseconds(100));

        switch (ret){
        case 0:
            stream.log("clear_tutorial: Detected tutorial screen.");
            seen_tutorial = true;
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        default:
            stream.log("clear_tutorial: Timed out.");
            if(!seen_tutorial){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "clear_tutorial(): Tutorial screen never detected.",
                    stream
                );                
            }
            return;
        }
    }
}

void clear_dialog(VideoStream& stream, ProControllerContext& context,
    ClearDialogMode mode, uint16_t seconds_timeout,
    std::vector<CallbackEnum> enum_optional_callbacks
){
    bool seen_dialog = false;
    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::minutes(3)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "clear_dialog(): Failed to clear dialog after 3 minutes.",
                stream
            );
        }

        AdvanceDialogWatcher    advance_dialog(COLOR_RED);
        OverworldWatcher    overworld(stream.logger(), COLOR_CYAN);
        PromptDialogWatcher prompt(COLOR_YELLOW, {0.50, 0.30, 0.40, 0.60});
        WhiteButtonWatcher  whitebutton(COLOR_GREEN, WhiteButton::ButtonA_DarkBackground, {0.725, 0.833, 0.024, 0.045}); // {0.650, 0.650, 0.140, 0.240}
        DialogArrowWatcher dialog_arrow(COLOR_RED, stream.overlay(), {0.850, 0.820, 0.020, 0.050}, 0.8365, 0.846);
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
            stream, context,
            std::chrono::seconds(seconds_timeout),
            callbacks
        );
        // int ret = run_until<ProControllerContext>(
        //     console, context,
        //     [&](ProControllerContext& context){
        //         for (size_t j = 0; j < seconds_timeout/3; j++){
        //             pbf_press_button(context, BUTTON_A, 20, 3*TICKS_PER_SECOND-20);
        //         }
        //     },
        //     {overworld, prompt, whitebutton, advance_dialog, battle}
        // );
        context.wait_for(std::chrono::milliseconds(100));
        if (ret < 0){
            stream.log("clear_dialog(): Timed out.");
            if (seen_dialog && mode == ClearDialogMode::STOP_TIMEOUT){
                return;
            }
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "clear_dialog(): Timed out. Did not detect dialog or did not detect the expected stop condition.",
                stream
            );
        }

        CallbackEnum enum_callback = enum_all_callbacks[ret];
        switch(enum_callback){
        case CallbackEnum::ADVANCE_DIALOG:
            stream.log("clear_dialog: Detected advance dialog.");
            seen_dialog = true;
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;            
        case CallbackEnum::OVERWORLD:
            stream.log("clear_dialog: Detected overworld.");
            if (seen_dialog && mode == ClearDialogMode::STOP_OVERWORLD){
                return;
            }
            break;
        case CallbackEnum::PROMPT_DIALOG:
            stream.log("clear_dialog: Detected prompt.");
            seen_dialog = true;
            if (mode == ClearDialogMode::STOP_PROMPT){
                return;
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case CallbackEnum::WHITE_A_BUTTON:
            stream.log("clear_dialog: Detected white A button.");
            seen_dialog = true;
            if (mode == ClearDialogMode::STOP_WHITEBUTTON){
                return;
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case CallbackEnum::DIALOG_ARROW:
            stream.log("clear_dialog: Detected dialog arrow.");
            seen_dialog = true;
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case CallbackEnum::BATTLE:
            stream.log("clear_dialog: Detected battle.");
            if (mode == ClearDialogMode::STOP_BATTLE){
                return;
            }
            break;
        case CallbackEnum::TUTORIAL:    
            stream.log("clear_dialog: Detected tutorial.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case CallbackEnum::BLACK_DIALOG_BOX:    
            stream.log("clear_dialog: Detected black dialog box.");
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
    VideoStream& stream,
    ProControllerContext& context
){
    while (true){
        DestinationMarkerWatcher marker(COLOR_RED, {0.815, 0.645, 0.180, 0.320}, true);
        NormalBattleMenuWatcher battle(COLOR_BLUE);

        int ret = wait_until(
            stream, context,
            std::chrono::seconds(5),
            {marker, battle}
        );
        switch (ret){
        case 0: // marker
            stream.log("Confirmed that marker is still present.");
            return true;
        case 1: // battle
            throw_and_log<UnexpectedBattleException>(
                stream.logger(), ErrorReport::SEND_ERROR_REPORT,
                "confirm_marker_present(): Unexpectedly detected battle.",
                stream
            );
        default:
            stream.log("Destination marker not detected.");
            return false;
        }   
    }

}

void overworld_navigation(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
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

        int ret = run_until<ProControllerContext>(
            stream, context,
            [&](ProControllerContext& context){

                for (int i = 0; i < seconds_timeout / seconds_realign; i++){
                    if (movement_mode == NavigationMovementMode::CLEAR_WITH_LETS_GO){
                        walk_forward_while_clear_front_path(info, stream, context, forward_ticks, y);
                    }else{
                        ssf_press_left_joystick(context, x, y, 0, seconds_realign * TICKS_PER_SECOND);
                        if (movement_mode == NavigationMovementMode::DIRECTIONAL_ONLY){
                            pbf_wait(context, seconds_realign * TICKS_PER_SECOND);
                        } else if (movement_mode == NavigationMovementMode::DIRECTIONAL_SPAM_A){
                            for (size_t j = 0; j < 5 * seconds_realign; j++){
                                pbf_press_button(context, BUTTON_A, 20, 5);
                            }
                        }
                    }
                    context.wait_for_all_requests();
                    if (should_realign){
                        try {
                            realign_player(info, stream, context, PlayerRealignMode::REALIGN_OLD_MARKER);
                   
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
            stream.log("overworld_navigation: Detected start of battle.");
            if (stop_condition == NavigationStopCondition::STOP_BATTLE){
                return;
            }

            run_wild_battle_press_A(stream, context, BattleStopCondition::STOP_OVERWORLD, {}, detect_wipeout);
            if (auto_heal){
                auto_heal_from_menu_or_overworld(info, stream, context, 0, true);
            }
            context.wait_for_all_requests();
            try {
                realign_player(info, stream, context, PlayerRealignMode::REALIGN_OLD_MARKER);
                if (stop_condition == NavigationStopCondition::STOP_MARKER && !confirm_marker_present(info, stream, context)){
                    // if marker not present when using marker based navigation, don't keep walking forward.
                    return;
                }

                break;
            }catch (UnexpectedBattleException&){
                break;
            }
        case 1: // dialog
            stream.log("overworld_navigation: Detected dialog.");
            if (stop_condition == NavigationStopCondition::STOP_DIALOG){
                return;
            }
            if (stop_condition == NavigationStopCondition::STOP_MARKER){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "overworld_navigation(): Unexpectedly detected dialog.",
                    stream
                );
            }          
            pbf_press_button(context, BUTTON_A, 20, 20);
            break;
        case 2: // marker
            stream.log("overworld_navigation: Detected marker.");
            if (stop_condition == NavigationStopCondition::STOP_MARKER){
                return;
            }
            break;
        default:
            stream.log("overworld_navigation(): Timed out.");
            if (stop_condition == NavigationStopCondition::STOP_TIME){
                return;
            }
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "overworld_navigation(): Timed out. Did not detect expected stop condition.",
                stream
            );
        }
    }
}

void config_option(ProControllerContext& context, int change_option_value){
    for (int i = 0; i < change_option_value; i++){
        pbf_press_dpad(context, DPAD_RIGHT, 13, 20);
    }
    pbf_press_dpad(context, DPAD_DOWN,  13, 20);
}

void swap_starter_moves(SingleSwitchProgramEnvironment& env, ProControllerContext& context, Language language){
    const ProgramInfo& info = env.program_info();
    VideoStream& stream = env.console;

    // start in the overworld
    press_Bs_to_back_to_overworld(info, stream, context);

    // open menu, select your starter
    enter_menu_from_overworld(info, stream, context, 0, MenuSide::LEFT);

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
    VideoSnapshot screen = stream.video().snapshot();
    PokemonMovesReader reader(language);
    std::string top_move = reader.read_move(stream.logger(), screen, 0);
    stream.log("Current top move: " + top_move);
    if (top_move != "ember" && top_move != "leafage" && top_move != "water-gun"){
        stream.log("Unable to confirm that the moves actually swapped.");
        OperationFailedException exception(
            ErrorReport::SEND_ERROR_REPORT,
            "swap_starter_moves: Unable to confirm that the moves actually swapped.\n" + language_warning(language),
            stream
        );
        exception.send_recoverable_notification(env);
    }   

}


void change_settings_prior_to_autostory(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    size_t current_segment_num,
    Language language
){

    // get index of `Options` in the Main Menu, which depends on where you are in Autostory
    int8_t options_index;  
    std::string assumption_text = "";
    switch(current_segment_num){
        case 0:
            return; // can't change settings in the intro cutscene
        case 1:
        // after Intro cutscene done, in room
            // Menu
            // - Options
            // - Save        
            options_index = 0;
            assumption_text = "We assume 'Bag' is not yet unlocked.";
            break;
        case 2:
            // Menu
            // - Bag  --> unlocked after picked up bag/hat in room. Segment 01, checkpoint 02
            // - Options
            // - Save
            options_index = 1;
            assumption_text = "We assume 'Boxes' is not yet unlocked.";
            break;
        case 3:
        case 4:
        case 5:
        case 6:
            // Menu
            // - Bag
            // - Boxes --> unlocked after battling Nemona and receiving Pokedex app. Segment 02, checkpoint 04
            // - Options
            // - Save        
            options_index = 2;
            assumption_text = "We assume 'Poke Portal' is not yet unlocked.";
            break;
        case 7:
        case 8:
        case 9:
            // Menu
            // - Bag
            // - Boxes
            // - Poke Portal --> unlocked after arriving at Los Platos and talking to Nemona. Segment 06, checkpoint 11
            // - Options
            // - Save  
            options_index = 3;
            assumption_text = "We assume 'Picnic' is not yet unlocked.";
            break;                    
        default:
            // Menu
            // - Bag
            // - Boxes
            // - Picnic --> unlocked after finishing tutorial. Segment 09, checkpoint 20
            // - Poke Portal
            // - Options
            // - Save          
            options_index = 4;
            assumption_text = "We assume that the tutorial is done, and all menu items are unlocked.";
            break;
    }
    
    env.console.log("change_settings_prior_to_autostory: " + assumption_text + " The index of \"Options\" in the Menu is " + std::to_string(options_index) + ".");
        
    bool has_minimap = current_segment_num >= 2;  // the minimap only shows up in segment 2 and beyond

    enter_menu_from_overworld(env.program_info(), env.console, context, options_index, MenuSide::RIGHT, has_minimap);
    change_settings(env, context, language);
    if(!has_minimap){
        pbf_mash_button(context, BUTTON_B, 2 * TICKS_PER_SECOND);
    }else{
        press_Bs_to_back_to_overworld(env.program_info(), env.console, context);    
    }
}


void change_settings(SingleSwitchProgramEnvironment& env, ProControllerContext& context,  Language language, bool use_inference){
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
    VideoStream& stream,
    ProControllerContext& context,
    std::function<
        void(const ProgramInfo& info, 
        VideoStream& stream,
        ProControllerContext& context)
    >&& action
){
    NormalBattleMenuWatcher battle_menu(COLOR_RED);
    int ret = run_until<ProControllerContext>(
        stream, context,
        [&](ProControllerContext& context){
            context.wait_for_all_requests();
            action(info, stream, context);
        },
        {battle_menu}
    );
    if (ret == 0){ // battle detected
        throw_and_log<UnexpectedBattleException>(
            stream.logger(), ErrorReport::SEND_ERROR_REPORT,
            "do_action_and_monitor_for_battles(): Detected battle. Failed to complete action.",
            stream
        );

    }
}


void handle_unexpected_battles(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    std::function<
        void(const ProgramInfo& info, 
        VideoStream& stream,
        ProControllerContext& context)
    >&& action
){
    while (true){
        try {
            context.wait_for_all_requests();
            action(info, stream, context);
            return;
        }catch (UnexpectedBattleException&){
            run_wild_battle_press_A(stream, context, BattleStopCondition::STOP_OVERWORLD);
        }
    }
}

void handle_when_stationary_in_overworld(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    std::function<
        void(const ProgramInfo& info, 
        VideoStream& stream,
        ProControllerContext& context)
    >&& action,
    std::function<
        void(const ProgramInfo& info, 
        VideoStream& stream,
        ProControllerContext& context)
    >&& recovery_action,
    size_t seconds_stationary,
    uint16_t minutes_timeout, 
    size_t max_failures
){
    
    WallClock start = current_time();
    size_t num_failures = 0;
    while (true){
        if (current_time() - start > std::chrono::minutes(minutes_timeout)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "handle_when_stationary_in_overworld(): Failed to complete action after " + std::to_string(minutes_timeout) + " minutes.",
                stream
            );
        }
        StationaryOverworldWatcher stationary_overworld(COLOR_RED, {0.865, 0.825, 0.08, 0.1}, seconds_stationary);

        int ret = run_until<ProControllerContext>(
            stream, context,
            [&](ProControllerContext& context){
                context.wait_for_all_requests();
                action(info, stream, context);
            },
            {stationary_overworld}        
        );
        if (ret < 0){
            // successfully completed action without being stuck in a position where the overworld is stationary.
            return;
        }else if (ret == 0){
            // if stationary in overworld, run recovery action then try action again
            stream.log("Detected stationary overworld.");
            num_failures++;
            if (num_failures > max_failures){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "handle_when_stationary_in_overworld(): Failed to complete action within " + std::to_string(max_failures) + " attempts.",
                    stream
                );                
            }
            context.wait_for_all_requests();
            recovery_action(info, stream, context);
        }
    }
}


void handle_failed_action(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    std::function<
        void(const ProgramInfo& info, 
        VideoStream& stream,
        ProControllerContext& context)
    >&& action,
    std::function<
        void(const ProgramInfo& info, 
        VideoStream& stream,
        ProControllerContext& context)
    >&& recovery_action,
    size_t max_failures
){
    size_t num_failures = 0;
    while (true){
        try {
            context.wait_for_all_requests();
            action(info, stream, context);
            return;
        }catch (OperationFailedException& e){
            num_failures++;
            if (num_failures > max_failures){
                throw e;
            }
            recovery_action(info, stream, context);
        }
    }
}


void wait_for_gradient_arrow(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context, 
    ImageFloatBox box_area_to_check,
    uint16_t seconds_timeout
){
    context.wait_for_all_requests();
    GradientArrowWatcher arrow(COLOR_RED, GradientArrowType::RIGHT, box_area_to_check);
    int ret = wait_until(
        stream, context,
        Milliseconds(seconds_timeout * 1000),
        { arrow }
    );
    if (ret == 0){
        stream.log("Gradient arrow detected.");
    }else{
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to detect gradient arrow.",
            stream
        );
    }          
}

void wait_for_overworld(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context, 
    uint16_t seconds_timeout
){
    context.wait_for_all_requests();
    OverworldWatcher        overworld(stream.logger(), COLOR_CYAN);
    int ret = wait_until(
        stream, context,
        Milliseconds(seconds_timeout * 1000),
        { overworld }
    );
    if (ret == 0){
        stream.log("Overworld detected.");
    }else{
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to detect overworld.",
            stream
        );
    }     

}

void press_A_until_dialog(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    uint16_t seconds_between_button_presses
){
    context.wait_for_all_requests();
    AdvanceDialogWatcher advance_dialog(COLOR_RED);
    int ret = run_until<ProControllerContext>(
        stream, context,
        [seconds_between_button_presses](ProControllerContext& context){
            pbf_wait(context, seconds_between_button_presses * TICKS_PER_SECOND); // avoiding pressing A if dialog already present
            for (size_t c = 0; c < 10; c++){
                pbf_press_button(context, BUTTON_A, 20, seconds_between_button_presses * TICKS_PER_SECOND);
            }
        },
        {advance_dialog}
    );
    if (ret == 0){
        stream.log("press_A_until_dialog: Detected dialog.");
    }else{
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "press_A_until_dialog(): Unable to detect dialog after 10 button presses.",
            stream
        );
    }
}

bool is_ride_active(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    while (true){
        try {
            // open main menu
            enter_menu_from_overworld(info, stream, context, -1, MenuSide::NONE, true);
            context.wait_for_all_requests();
            ImageStats ride_indicator = image_stats(
                extract_box_reference(
                    stream.video().snapshot(),
                    ImageFloatBox(0.05, 0.995, 0.25, 0.003)
                )
            );

            bool is_ride_active = !is_black(ride_indicator); // ride is active if the ride indicator isn't black.
            pbf_press_button(context, BUTTON_B, 30, 100);
            press_Bs_to_back_to_overworld(info, stream, context, 7);
            if (is_ride_active){
                stream.log("Ride is active.");
            }else{
                stream.log("Ride is not active.");
            }
            return is_ride_active;        

        }catch(UnexpectedBattleException&){
            run_wild_battle_press_A(stream, context, BattleStopCondition::STOP_OVERWORLD);
        }
    }

}

void get_on_ride(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    get_on_or_off_ride(info, stream, context, true);
}

void get_off_ride(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    get_on_or_off_ride(info, stream, context, false);
}

void get_on_or_off_ride(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context, bool get_on){
    pbf_press_button(context, BUTTON_PLUS, 20, 20);

    WallClock start = current_time();
    while (get_on != is_ride_active(info, stream, context)){
        if (current_time() - start > std::chrono::minutes(3)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "get_on_or_off_ride(): Failed to get on/off ride after 3 minutes.",
                stream
            );
        }        
        pbf_press_button(context, BUTTON_PLUS, 30, 100);
    }
}

void checkpoint_save(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    save_game_from_overworld(env.program_info(), env.console, context);
    stats.m_checkpoint++;
    env.update_stats();
    send_program_status_notification(env, notif_status_update, "Saved at checkpoint.");        
}


void realign_player_from_landmark(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    MoveCursor move_cursor_near_landmark,
    MoveCursor move_cursor_to_target
){

    stream.log("Realigning player direction, using a landmark...");
    WallClock start = current_time();

    const int MAX_TRY_COUNT = 17;
    int try_count = 0;

    // failures to fly to pokecenter are often when the Switch lags. from my testing, a 1.4-1.5 adjustment factor seems to work
    const std::array<double, MAX_TRY_COUNT> adjustment_table =  {1, 1.4, 1, 1, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0, 0.9, 0.8, 1.4}; // {1, 1.4, 1.5};

    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "realign_player_from_landmark(): Failed to realign player after 5 minutes.",
                stream
            );
        }

        try {
            open_map_from_overworld(info, stream, context, false);

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
            double push_scale = 0.29 * adjustment_table[try_count];
            if (!detect_closest_flypoint_and_move_map_cursor_there(info, stream, context, FlyPoint::POKECENTER, push_scale)){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "realign_player_from_landmark(): No visible pokecenter found on map.",
                    stream
                );         
            }

            confirm_cursor_centered_on_pokecenter(info, stream, context); // throws exception if fails

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
            leave_phone_to_overworld(info, stream, context);

            return;      

        }catch (UnexpectedBattleException&){
            run_wild_battle_press_A(stream, context, BattleStopCondition::STOP_OVERWORLD);
        }catch (OperationFailedException&){
            try_count++;
            if (try_count >= MAX_TRY_COUNT){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "fly_to_closest_pokecenter_on_map(): At min warpable map level, pokecenter was detected, but failed to fly there.",
                    stream
                );                
            }
            stream.log("Failed to find the fly menu item. Restart the closest Pokecenter travel process.");
            press_Bs_to_back_to_overworld(info, stream, context);
        }
    }
    

}


void confirm_cursor_centered_on_pokecenter(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();
    context.wait_for(Milliseconds(500));
    ImageFloatBox center_cursor{0.484, 0.472, 0.030, 0.053};
    MapPokeCenterIconDetector pokecenter(COLOR_RED, center_cursor);
    if (!pokecenter.detect(stream.video().snapshot())){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "confirm_cursor_centered_on_pokecenter(): Cursor is not centered on a pokecenter.",
            stream
        );            
    }

    stream.log("Confirmed that the cursor is centered on a pokecenter.");
}

void move_cursor_towards_flypoint_and_go_there(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    MoveCursor move_cursor_near_flypoint,
    FlyPoint fly_point
){
    WallClock start = current_time();

    const int MAX_TRY_COUNT = 17;
    int try_count = 0;
    
    // failures to fly to pokecenter are often when the Switch lags. from my testing, a 1.4-1.5 adjustment factor seems to work
    const std::array<double, MAX_TRY_COUNT> adjustment_table =  {1, 1.4, 1, 1, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0, 0.9, 0.8, 1.4}; // {1, 1.4, 1.5};

    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "move_cursor_towards_flypoint_and_go_there(): Failed to fly after 5 minutes.",
                stream
            );
        }

        try {
            open_map_from_overworld(info, stream, context, false);

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

            double push_scale = 0.29 * adjustment_table[try_count];
            if (!fly_to_visible_closest_flypoint_cur_zoom_level(info, stream, context, fly_point, push_scale)){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "move_cursor_towards_flypoint_and_go_there(): No visible pokecenter found on map.",
                    stream
                );                  
            }

            return;      

        }catch (UnexpectedBattleException&){
            run_wild_battle_press_A(stream, context, BattleStopCondition::STOP_OVERWORLD);
        }catch (OperationFailedException&){
            try_count++;
            if (try_count >= MAX_TRY_COUNT){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "move_cursor_towards_flypoint_and_go_there(): At given zoom level, pokecenter was detected, but failed to fly there.",
                    stream
                );                
            }
            stream.log("Failed to find the fly menu item. Restart the closest Pokecenter travel process.");
            press_Bs_to_back_to_overworld(info, stream, context);
        }
    }
    

}



void check_num_sunflora_found(SingleSwitchProgramEnvironment& env, ProControllerContext& context, int expected_number){
    context.wait_for_all_requests();
    VideoSnapshot screen = env.console.video().snapshot();
    ImageFloatBox num_sunflora_box = {0.27, 0.02, 0.04, 0.055};
    int number = OCR::read_number_waterfill(env.console, extract_box_reference(screen, num_sunflora_box), 0xff000000, 0xff808080);

    if (number != expected_number){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "The number of sunflora found is different than expected.",
            env.console
        );
    }else{
        env.console.log("Number of sunflora found: " + std::to_string(number));
    }


}

void checkpoint_reattempt_loop(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats,
    std::function<void(size_t attempt_number)>&& action
){
    size_t max_attempts = 100;
    for (size_t i = 0;;i++){
    try{
        if (i==0){
            checkpoint_save(env, context, notif_status_update, stats);
        }

        context.wait_for_all_requests();
        action(i);
       
        break;
    }catch(OperationFailedException& e){
        if (i > max_attempts){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Autostory checkpoint failed " + std::to_string(max_attempts) + " times.\n"
                "Make sure you selected the correct Start Point, and your character is in the exactly correct starting position."
                "Also, make sure you have set the correct Language.\n" + e.message(),
                env.console
            );
        }
        context.wait_for_all_requests();
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }         
    }

}

void checkpoint_reattempt_loop_tutorial(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats,
    std::function<void(size_t attempt_number)>&& action
){
    size_t max_attempts = 100;
    for (size_t i = 0;;i++){
    try{
        if(i==0){
            save_game_tutorial(env.program_info(), env.console, context);
            stats.m_checkpoint++;
            env.update_stats();
            send_program_status_notification(env, notif_status_update, "Saved at checkpoint.");     
        }
        
        context.wait_for_all_requests();
        action(i);

        break;  
    }catch(OperationFailedException& e){
        if (i > max_attempts){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Autostory checkpoint failed " + std::to_string(max_attempts) + " times.\n"
                "Make sure you selected the correct Start Point, and your character is in the exactly correct starting position."
                "Also, make sure you have set the correct Language.\n" + e.message(),
                env.console
            );
        }        
        context.wait_for_all_requests();
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }
    }    
}



void move_from_glaseado_mountain_to_casseroya_watchtower3(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();

    DirectionDetector direction;
    direction.change_direction(env.program_info(), env.console, context, 1.448679);

    pbf_move_left_joystick(context, 128, 0, 200, 50);


    // marker 1   x=0.548438, y=0.273148
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_OUT, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.548438, 0.273148}
    );

    get_on_ride(env.program_info(), env.console, context);

    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 2.    :  x=0.693229, y=0.459259
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.693229, 0.459259}
    );        
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    fly_to_overlapping_flypoint(env.program_info(), env.console, context); 

}


void move_from_casseroya_watchtower3_to_dondozo_titan(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();
    
    // marker 1     x=0.779167, y=0.274074
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.779167, 0.274074}
    );

    get_on_ride(env.program_info(), env.console, context);

    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    mash_button_till_overworld(env.console, context, BUTTON_A);

    // resume marker 1
    realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 2.    :   x=0.76875, y=0.298148
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::FAST_TRAVEL, 
        {0.76875, 0.298148}
    );        
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );


    // marker 3.    :   x=0.752604, y=0.401852
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::FAST_TRAVEL, 
        {0.752604, 0.401852}
    );        
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_SPAM_A, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE});
    env.console.log("Battle Dondozo/Tatsugiri Titan phase 1.");
    run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);


}

void move_from_dondozo_titan_phase1_to_phase2(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();
    
    // marker 1      x=0.832292, y=0.54537
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::FAST_TRAVEL, 
        {0.832292, 0.54537}
    );

    get_on_ride(env.program_info(), env.console, context);

    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 2       x=0.393229, y=0.748148
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, 0, 0, 0}, 
        FlyPoint::FAST_TRAVEL, 
        {0.393229, 0.748148}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );


    // marker 3. go to Dondozo/Tatsugiri part 2       x=0.55625, y=0.324074
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 255, 100}, 
        FlyPoint::FAST_TRAVEL, 
        {0.55625, 0.324074}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 80, 40, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE});
    
    env.console.log("Battle Dondozo/Tatsugiri Titan phase 2.");
    run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
    clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE});
    
    env.console.log("Battle Dondozo/Tatsugiri Titan phase 3.");
    run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG, {CallbackEnum::DIALOG_ARROW});
    mash_button_till_overworld(env.console, context, BUTTON_A, 360);

}

void move_from_glaseado_mountain_to_montenevera(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();


    // marker 1         {0.745313, 0.637037}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.745313, 0.637037}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );
    
    // marker 2       x=0.225521, y=0.380556. {0.229687, 0.37037}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 255, 0, 50}, 
        FlyPoint::POKECENTER, 
        {0.229687, 0.37037}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 60, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 3       x=0.396354, y=0.69537.  x=0.396875, y=0.725926
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 255, 200, 50}, 
        FlyPoint::POKECENTER, 
        {0.396875, 0.725926}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 50, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 4       (0.409896, 0.65)
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.409896, 0.65}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    
    // marker 5        {0.509896, 0.639815}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.509896, 0.639815}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 6         {0.508333, 0.566667}   {0.521875, 0.483333}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, 128, 255, 20}, 
        FlyPoint::POKECENTER, 
        {0.508333, 0.566667}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 7         {0.508333, 0.566667}   {0.521875, 0.483333}  {0.529687, 0.483333}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.529687, 0.483333}
    );

    get_on_ride(env.program_info(), env.console, context);

    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 128, 0, 500ms, 0ms);
            pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 1000ms);
            pbf_move_left_joystick(context, 128, 0, 500ms, 0ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );


    fly_to_overlapping_flypoint(env.program_info(), env.console, context); 


}


void move_from_montenevera_to_glaseado_gym(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DirectionDetector direction;
    direction.change_direction(env.program_info(), env.console, context, 1.255489);
    pbf_move_left_joystick(context, 128, 0, 400, 50);

    direction.change_direction(env.program_info(), env.console, context, 2.463760);
    pbf_move_left_joystick(context, 128, 0, 1100, 50);

     
    // marker 1 {0.585938, 0.236111}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 128, 255, 20}, 
        FlyPoint::POKECENTER, 
        {0.585938, 0.236111}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );


    // marker 2 cross bridge   {0.719271, 0.585185}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 128, 255, 30}, 
        FlyPoint::POKECENTER, 
        {0.719271, 0.585185}
    );

    get_on_ride(env.program_info(), env.console, context);

    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 128, 0, 500ms, 0ms);
            pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 1000ms);
            pbf_move_left_joystick(context, 128, 0, 500ms, 0ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    get_off_ride(env.program_info(), env.console, context);

    // marker 3           {0.73125, 0.481481}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 128, 255, 30}, 
        FlyPoint::POKECENTER, 
        {0.73125, 0.481481}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){ // todo: get on ride?
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );


    // marker 4     {0.691146, 0.347222}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.691146, 0.347222}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 5      {0.632292, 0.376852}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.632292, 0.376852}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );


    // marker 6     {0.61875, 0.432407}       {0.610417, 0.42037}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.610417, 0.42037}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 7          {0.613542, 0.540741}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.613542, 0.540741}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 8          {0.588021, 0.578704}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.588021, 0.578704}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 9         {0.579167, 0.610185}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.579167, 0.610185}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 16, 8, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 10. end up next to Pokecenter        {0.520833, 0.443519}     {0.490625, 0.4}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.490625, 0.4}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );


    // marker 11. set marker past pokecenter
    handle_unexpected_battles(env.program_info(), env.console, context,
    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 210, 0, 50);
    });      
    overworld_navigation(env.program_info(), env.console, context, 
        NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
        128, 15, 12, 12, false);           // can't wrap in handle_when_stationary_in_overworld(), since we expect to be stationary when walking into the pokecenter
        

    fly_to_overlapping_flypoint(env.program_info(), env.console, context); 



}

void move_from_glaseado_mountain_to_north_province_area_three(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    
    context.wait_for_all_requests();

    // marker 1. set marker to pokecenter
    realign_player_from_landmark(
        env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 128, 0, 50},
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}
    );  
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 80, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    ); 


    // marker 2. set marker past pokecenter
    handle_unexpected_battles(env.program_info(), env.console, context,
    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 128, 0, 50);
    });      
    overworld_navigation(env.program_info(), env.console, context, 
        NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
        128, 15, 12, 12, false);           // can't wrap in handle_when_stationary_in_overworld(), since we expect to be stationary when walking into the pokecenter
        

    fly_to_overlapping_flypoint(env.program_info(), env.console, context); 

}


void move_from_glaseado_gym_to_north_province_area_one(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();

    DirectionDetector direction;
    handle_unexpected_battles(env.program_info(), env.console, context,
    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){

        direction.change_direction(env.program_info(), env.console, context, 3.855289);
        pbf_move_left_joystick(context, 128, 0, 200, 50);

        direction.change_direction(env.program_info(), env.console, context, 3.056395);
        pbf_move_left_joystick(context, 128, 0, 300, 50);

        direction.change_direction(env.program_info(), env.console, context, 3.805047);
        pbf_move_left_joystick(context, 128, 0, 700, 50);

        direction.change_direction(env.program_info(), env.console, context, 1.589021);
        pbf_move_left_joystick(context, 128, 0, 600, 50);
        
        direction.change_direction(env.program_info(), env.console, context, 2.741829);
        pbf_move_left_joystick(context, 128, 0, 300, 50);
    });

    // marker 1     {0.33125, 0.314815}      {0.297396, 0.322222}        {0.313021, 0.322222}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.313021, 0.322222}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 2         {0.222396, 0.413889}         {0.253646, 0.386111}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.253646, 0.386111}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );
        
 

    // marker 3   {0.80625, 0.574074}        {0.803125, 0.615741}       {0.794792, 0.614815}        (0.794792, 0.609259).
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 255, 0, 50}, 
        FlyPoint::POKECENTER, 
        {0.794792, 0.609259}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 50, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 4. set marker to pokecenter
    realign_player_from_landmark(
        env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 255, 128, 80},
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}
    );  
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 110, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    ); 

    // marker 5. set marker past pokecenter
    handle_unexpected_battles(env.program_info(), env.console, context,
    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 255, 50);
    });      
    overworld_navigation(env.program_info(), env.console, context, 
        NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
        128, 15, 12, 12, false);           // can't wrap in handle_when_stationary_in_overworld(), since we expect to be stationary when walking into the pokecenter
        

    fly_to_overlapping_flypoint(env.program_info(), env.console, context); 


}


void move_from_north_province_area_one_to_fighting_base(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DirectionDetector direction;
    handle_unexpected_battles(env.program_info(), env.console, context,
    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){

        direction.change_direction(env.program_info(), env.console, context, 1.798578);
        pbf_move_left_joystick(context, 128, 0, 200, 50);

        
    });

    // marker 1     {0.825, 0.361111}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 255, 255, 30}, 
        FlyPoint::POKECENTER, 
        {0.825, 0.361111}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 2             {0.839062, 0.267593}  {0.834896, 0.267593}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.834896, 0.267593}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

 

    // marker 3             {0.764583, 0.244444}            (0.775000, 0.250000).
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.775000, 0.250000}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 4    {0.604167, 0.326852}    {0.597396, 0.32037}    {0.600521, 0.325}    {0.589583, 0.319444}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_OUT, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.597396, 0.32037}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 5    enter bamboo forest      keep zoom{0.679688, 0.19537}   zoom out  {0.571875, 0.360185}   
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_OUT, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.571875, 0.360185}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 6     {0.668229, 0.336111}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 128, 0, 20}, 
        FlyPoint::POKECENTER, 
        {0.668229, 0.336111}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 7      {0.496354, 0.20463}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, 255, 100, 80}, 
        FlyPoint::POKECENTER, 
        {0.496354, 0.20463}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 40, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 8      {0.428125, 0.483333}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, 255, 0, 50}, 
        FlyPoint::POKECENTER, 
        {0.428125, 0.483333}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 30, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // speak to Clavell at Fighting base
    mash_button_till_overworld(env.console, context, BUTTON_A, 360);


}


void beat_team_star_fighting1(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

    realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 220, 50);

    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::PROMPT_DIALOG, CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW});
    env.console.log("Battle team star grunt.");
    run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
    mash_button_till_overworld(env.console, context, BUTTON_A);




}

void beat_team_star_fighting2(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();
    do_action_and_monitor_for_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 220, 255, 50);
            walk_forward_while_clear_front_path(env.program_info(), env.console, context, 100);
            walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A);
        }
    );
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD, CallbackEnum::PROMPT_DIALOG});


    AdvanceDialogWatcher    dialog(COLOR_RED);
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){

            DirectionDetector direction;
            uint16_t seconds_wait = 3; 

            direction.change_direction(env.program_info(), env.console, context, 3.837821);
            pbf_move_left_joystick(context, 128, 0, 400, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 300, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 300, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 3.577868);
            pbf_move_left_joystick(context, 128, 0, 500, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 500, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 200, 50);

            direction.change_direction(env.program_info(), env.console, context, 4.103129);
            pbf_move_left_joystick(context, 128, 0, 300, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 300, 50);

            direction.change_direction(env.program_info(), env.console, context, 3.316950);
            pbf_move_left_joystick(context, 128, 0, 300, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);


            direction.change_direction(env.program_info(), env.console, context, 3.472393);
            pbf_move_left_joystick(context, 128, 0, 700, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 3.039084);
            pbf_move_left_joystick(context, 128, 0, 400, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);


            direction.change_direction(env.program_info(), env.console, context, 3.839029);
            pbf_move_left_joystick(context, 128, 0, 200, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 500, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 4.048405);
            pbf_move_left_joystick(context, 128, 0, 400, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);


            pbf_move_left_joystick(context, 128, 0, 200, 50);

            direction.change_direction(env.program_info(), env.console, context, 4.624776);
            pbf_move_left_joystick(context, 128, 0, 200, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 300, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 6.035424);
            pbf_move_left_joystick(context, 128, 0, 25, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 400, 50);

            direction.change_direction(env.program_info(), env.console, context,  5.620459);
            pbf_move_left_joystick(context, 128, 0, 200, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context,  6.038329);
            pbf_move_left_joystick(context, 128, 0, 200, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);
            
            pbf_move_left_joystick(context, 128, 0, 300, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context,  6.281440);
            pbf_move_left_joystick(context, 128, 0, 300, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 5.565887);
            pbf_move_left_joystick(context, 128, 0, 300, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 5.515809);
            pbf_move_left_joystick(context, 128, 0, 300, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 200, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 0.889223);
            pbf_move_left_joystick(context, 128, 0, 200, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);


            direction.change_direction(env.program_info(), env.console, context,  2.934897);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_move_left_joystick(context, 128, 0, 200, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 2.617821);
            pbf_move_left_joystick(context, 128, 0, 300, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 2.827481);
            pbf_move_left_joystick(context, 128, 0, 200, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 300, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 2.883368);
            pbf_move_left_joystick(context, 128, 0, 300, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 300, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 200, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 2.094273);
            pbf_move_left_joystick(context, 128, 0, 200, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 200, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 1.619823);
            pbf_move_left_joystick(context, 128, 0, 200, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 200, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 0.886674);
            pbf_move_left_joystick(context, 128, 0, 200, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 600, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 0.468837);
            pbf_move_left_joystick(context, 128, 0, 200, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

                
            
        },
        {dialog}
    );
    context.wait_for(std::chrono::milliseconds(100));
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "checkpoint_48(): Failed to kill 30 pokemon with Let's go.",
            env.console
        );            
    }
    clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW});
    env.console.log("Battle the Team Star (Fire) boss.");
    run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
    mash_button_till_overworld(env.console, context, BUTTON_A, 360);


}


void move_from_fighting_base_to_north_province_area_two(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();

    // marker 1. set marker to pokecenter
    realign_player_from_landmark(
        env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, 0, 0, 0},
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}
    );  
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    ); 

    // marker 2. set marker past pokecenter
    handle_unexpected_battles(env.program_info(), env.console, context,
    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 60, 50);
    });      
    overworld_navigation(env.program_info(), env.console, context, 
        NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
        128, 15, 12, 12, false);           // can't wrap in handle_when_stationary_in_overworld(), since we expect to be stationary when walking into the pokecenter
        

    fly_to_overlapping_flypoint(env.program_info(), env.console, context); 

}


void move_from_west_province_area_one_north_to_alfornada(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();
    DirectionDetector direction;

    handle_unexpected_battles(env.program_info(), env.console, context,
    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
        // marker 1
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 140, 255, 30);
        direction.change_direction(env.program_info(), env.console, context, 4.047990);
        pbf_move_left_joystick(context, 128, 0, 200, 50);
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
    });

    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    ); 

    // marker 2      zoom in{0.605729, 0.30463}, zoom out{0.684375, 0.616667}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_OUT, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.684375, 0.616667}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 24, 8, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 3  {0.767708, 0.45}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 255, 255, 60}, 
        FlyPoint::POKECENTER, 
        {0.767708, 0.45}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 60, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );


    // marker 4   {0.763021, 0.253704}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.763021, 0.253704}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 5  {0.780729, 0.216667}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.780729, 0.216667}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 6. place the marker at the top of the cliff. {0.633333, 0.304630}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_OUT, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.633333, 0.304630}
    );
    
    handle_unexpected_battles(env.program_info(), env.console, context,
    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
        // walk towards wall
        direction.change_direction(env.program_info(), env.console, context, 2.949863);

        walk_forward_while_clear_front_path(env.program_info(), env.console, context, 700);

        // back away from wall and get on ride
        pbf_move_left_joystick(context, 128, 255, 50, 50);
        get_on_ride(env.program_info(), env.console, context);

        // move back towards wall
        pbf_move_left_joystick(context, 128, 0, 100, 50);
        pbf_move_left_joystick(context, 0, 0, 400, 50);

        direction.change_direction(env.program_info(), env.console, context, 2.566167);

        // climb the wall
        pbf_press_button(context, BUTTON_B, 31ms, 0ms);
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 855ms);
        pbf_move_left_joystick(context, 128, 0, 7905ms, 0ms);
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 779ms);
        pbf_move_left_joystick(context, 128, 0, 325ms, 0ms);
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 657ms);
        pbf_move_left_joystick(context, 128, 0, 3271ms, 0ms);
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 860ms);
        pbf_move_left_joystick(context, 128, 0, 220ms, 0ms);
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 993ms);
        pbf_move_left_joystick(context, 128, 0, 3572ms, 0ms);

        get_off_ride(env.program_info(), env.console, context);

        // realign to marker 6
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        
    });

    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );


    // marker 7 {0.501042, 0.738889}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 200, 30}, 
        FlyPoint::POKECENTER, 
        {0.501042, 0.738889}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 8. set marker to pokecenter
    realign_player_from_landmark(
        env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 128, 255, 30},
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}
    ); 

    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 40, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 9. set marker past pokecenter
    handle_unexpected_battles(env.program_info(), env.console, context,
    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 120, 255, 50);
    });      
    overworld_navigation(env.program_info(), env.console, context, 
        NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
        128, 15, 12, 12, false);           // can't wrap in handle_when_stationary_in_overworld(), since we expect to be stationary when walking into the pokecenter
        

    fly_to_overlapping_flypoint(env.program_info(), env.console, context); 


    

}



}
}
}

