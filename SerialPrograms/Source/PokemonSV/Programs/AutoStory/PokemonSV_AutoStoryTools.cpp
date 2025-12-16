/*  AutoStoryTools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h" 
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
#include "PokemonSV/Inference/Overworld/PokemonSV_NoMinimapDetector.h"
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
    std::vector<CallbackEnum> enum_optional_callbacks,
    bool press_A
){
    bool seen_dialog = false;
    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "clear_dialog(): Failed to clear dialog after 5 minutes.",
                stream
            );
        }

        AdvanceDialogWatcher    advance_dialog(COLOR_RED);
        OverworldWatcher    overworld(stream.logger(), COLOR_CYAN);
        PromptDialogWatcher prompt(COLOR_YELLOW, {0.50, 0.25, 0.40, 0.65});
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


        // int ret = wait_until(
        //     stream, context,
        //     std::chrono::seconds(seconds_timeout),
        //     callbacks
        // );

        WallClock start_inference = current_time();
        int ret = run_until<ProControllerContext>(
            stream, context,
            [&](ProControllerContext& context){

                if (mode == ClearDialogMode::STOP_TIMEOUT || !press_A){
                    context.wait_for(Seconds(seconds_timeout));
                }else{ // press A every 25 seconds, until we time out.
                    auto button_press_period = Seconds(25);
                    while (true){
                        if (current_time() - start_inference > Seconds(seconds_timeout)){
                            break;
                        }
                        context.wait_for(button_press_period);
                        pbf_press_button(context, BUTTON_A, 160ms, 0ms);
                    }
                }
            },
            callbacks
        );
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
            if (mode == ClearDialogMode::STOP_BATTLE_DIALOG_ARROW){
                return;
            }
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
            if (mode == ClearDialogMode::STOP_TUTORIAL){
                return;
            }
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

void realign_player(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    PlayerRealignMode realign_mode,
    uint8_t move_x, uint8_t move_y, uint16_t move_duration
){
    stream.log("Realigning player direction...");
    switch (realign_mode){
    case PlayerRealignMode::REALIGN_NEW_MARKER:
        stream.log("Setting new map marker...");

        handle_unexpected_battles(info, stream, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            open_map_from_overworld(info, stream, context);
        });

        pbf_press_button(context, BUTTON_ZR, 20, 105);
        pbf_move_left_joystick(context, move_x, move_y, move_duration, 1 * TICKS_PER_SECOND);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_A, 20, 105);

        handle_unexpected_battles(info, stream, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){        
            leave_phone_to_overworld(info, stream, context);
        });
        return;     
    case PlayerRealignMode::REALIGN_OLD_MARKER:
        handle_unexpected_battles(info, stream, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            open_map_from_overworld(info, stream, context, false);
        });

        handle_unexpected_battles(info, stream, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){        
            leave_phone_to_overworld(info, stream, context);
        });

        pbf_press_button(context, BUTTON_L, 20, 105);
        return;
    case PlayerRealignMode::REALIGN_NO_MARKER:
        pbf_move_left_joystick(context, move_x, move_y, move_duration, 1 * TICKS_PER_SECOND);
        pbf_press_button(context, BUTTON_L, 20, 105);
        return;
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
        NoMinimapWatcher no_minimap(stream.logger(), COLOR_RED, Milliseconds(250));
        NormalBattleMenuWatcher battle(COLOR_BLUE);
        AdvanceDialogWatcher        dialog(COLOR_RED);
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
                    // if detect no minimap, then stop moving or spamming A.
                    int ret2 = run_until<ProControllerContext>(
                        stream, context,
                        [&](ProControllerContext& context){
                            if (movement_mode == NavigationMovementMode::CLEAR_WITH_LETS_GO){
                                walk_forward_while_clear_front_path(info, stream, context, forward_ticks, y);
                            }else{
                                ssf_press_left_joystick(context, x, y, 0ms, Seconds(seconds_realign));
                                if (movement_mode == NavigationMovementMode::DIRECTIONAL_ONLY){
                                    pbf_wait(context, Seconds(seconds_realign));
                                } else if (movement_mode == NavigationMovementMode::DIRECTIONAL_SPAM_A){
                                    for (size_t j = 0; j < 5 * seconds_realign; j++){
                                        pbf_press_button(context, BUTTON_A, 20, 5);
                                    }
                                }
                            }
                        },
                        { no_minimap }
                    );

                    if (ret2 == 0){
                        stream.log("overworld_navigation: No minimap detected. Wait for Battle or Dialog.");
                        context.wait_for(Seconds(30));
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


void confirm_lead_pokemon_moves(SingleSwitchProgramEnvironment& env, ProControllerContext& context, Language language){
    const ProgramInfo& info = env.program_info();
    VideoStream& stream = env.console;

    // start in the overworld
    press_Bs_to_back_to_overworld(info, stream, context);

    // open menu, select your lead pokemon
    enter_menu_from_overworld(info, stream, context, 0, MenuSide::LEFT);

    // enter Pokemon summary screen
    pbf_press_button(context, BUTTON_A, 20, 5 * TICKS_PER_SECOND);
    pbf_press_dpad(context, DPAD_RIGHT, 15, 1 * TICKS_PER_SECOND);
    pbf_press_button(context, BUTTON_Y, 20, 40);

    // confirm that moves are: Moonblast, Mystical Fire, Psychic, Misty Terrain
    context.wait_for_all_requests();
    VideoSnapshot screen = stream.video().snapshot();
    PokemonMovesReader reader(language);
    std::string move_0 = reader.read_move(stream.logger(), screen, 0);
    std::string move_1 = reader.read_move(stream.logger(), screen, 1);
    std::string move_2 = reader.read_move(stream.logger(), screen, 2);
    std::string move_3 = reader.read_move(stream.logger(), screen, 3);
    stream.log("Current first move: " + move_0);
    stream.log("Current second move: " + move_1);
    stream.log("Current third move: " + move_2);
    stream.log("Current fourth move: " + move_3);

    if (move_0 != "moonblast" || move_1 != "mystical-fire" || move_2 != "psychic" || move_3 != "misty-terrain"){
        stream.log("Lead Pokemon's moves are wrong. They are supposed to be: Moonblast, Mystical Fire, Psychic, Misty Terrain.");
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "We expect your lead Pokemon to be a Gardevoir with moves in this order: Moonblast, Mystical Fire, Psychic, Misty Terrain. "
            "But we see something else instead. If you confirm that your lead Gardevoir does indeed have these moves in this order, "
            "and are still getting this error, you can uncheck 'Pre-check: Ensure correct moves', under Advanced mode.\n" + language_warning(language),
            stream
        );
    }   

    press_Bs_to_back_to_overworld(info, stream, context);

}

void change_settings_prior_to_autostory_segment_mode(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t current_segment_num, Language language){
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
    change_settings_prior_to_autostory(env, context, options_index, has_minimap, language);
}

void change_settings_prior_to_autostory_checkpoint_mode(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t current_checkpoint_num, Language language){
    // get index of `Options` in the Main Menu, which depends on where you are in Autostory
    int8_t options_index;  
    std::string assumption_text = "";
    switch(current_checkpoint_num){
        case 0:
            return; // can't change settings in the intro cutscene
        case 1:
        case 2:
        // after Intro cutscene done, in room
            // Menu
            // - Options
            // - Save        
            options_index = 0;
            assumption_text = "We assume 'Bag' is not yet unlocked.";
            break;
        case 3:
        case 4:
            // Menu
            // - Bag  --> unlocked after picked up bag/hat in room. Segment 01, checkpoint 02
            // - Options
            // - Save
            options_index = 1;
            assumption_text = "We assume 'Boxes' is not yet unlocked.";
            break;
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
            // Menu
            // - Bag
            // - Boxes --> unlocked after battling Nemona and receiving Pokedex app. Segment 02, checkpoint 04
            // - Options
            // - Save        
            options_index = 2;
            assumption_text = "We assume 'Poke Portal' is not yet unlocked.";
            break;
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
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
            if(current_checkpoint_num <= 20){
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "change_settings_prior_to_autostory_checkpoint_mode: current_checkpoint_num should be greater than 20.");           
            }

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
        
    bool has_minimap = current_checkpoint_num >= 3;  // the minimap only shows up in checkpoint 3 and beyond
    change_settings_prior_to_autostory(env, context, options_index, has_minimap, language);
}


void change_settings_prior_to_autostory(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    int options_index,
    bool has_minimap,
    Language language
){

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

void do_action_and_monitor_for_battles_early(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    std::function<
        void(const ProgramInfo& info, 
        VideoStream& stream,
        ProControllerContext& context)
    >&& action
){
    NoMinimapWatcher no_minimap(stream.logger(), COLOR_RED, Milliseconds(250));
    int ret = run_until<ProControllerContext>(
        stream, context,
        [&](ProControllerContext& context){
            context.wait_for_all_requests();
            action(info, stream, context);
        },
        {no_minimap}
    );
    if (ret == 0){  // if see no minimap. stop and see if we detect a battle. if so, throw Battl exception
        do_action_and_monitor_for_battles(info, stream, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_wait(context, Seconds(30));
        });

        // if no battle seen, then throw Exception.
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "do_action_and_monitor_for_battles_early(): Expected to see a battle, but didn't. Possible false positive on NoMinimapWatcher.",
            stream
        ); 
    }

}


void do_action_until_dialog(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    std::function<
        void(const ProgramInfo& info, 
        VideoStream& stream,
        ProControllerContext& context)
    >&& action
){
    AdvanceDialogWatcher    dialog(COLOR_RED);
    int ret = run_until<ProControllerContext>(
        stream, context,
        [&](ProControllerContext& context){
            context.wait_for_all_requests();
            action(info, stream, context);
        },
        {dialog}
    );
    context.wait_for(std::chrono::milliseconds(100));

    switch (ret){
    case 0: // dialog
        stream.log("do_action_until_dialog(): Detected dialog.");
        return;
    default:
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "do_action_until_dialog(): Finished action. Did not detect dialog.",
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

void do_action_and_monitor_for_overworld(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    std::function<
        void(const ProgramInfo& info, 
        VideoStream& stream,
        ProControllerContext& context)
    >&& action
){

    OverworldWatcher overworld(stream.logger(), COLOR_CYAN);

    int ret = run_until<ProControllerContext>(
        stream, context,
        [&](ProControllerContext& context){
            context.wait_for_all_requests();
            action(info, stream, context);
        },
        {overworld}        
    );
    if (ret < 0){
        // successfully completed action detecting the overworld
        return;
    }else if (ret == 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "do_action_and_monitor_for_overworld(): Failed to complete action. Detected overworld.",
            stream
        );                
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
            pbf_wait(context, Seconds(seconds_between_button_presses)); // avoiding pressing A if dialog already present
            for (size_t c = 0; c < 10; c++){
                pbf_press_button(context, BUTTON_A, 20*8ms, Seconds(seconds_between_button_presses));
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
    std::function<void(size_t attempt_number)>&& action,
    bool day_skip
){
    size_t max_attempts = 100;
    for (size_t i = 0;;i++){
    try{
        if (i==0){
            checkpoint_save(env, context, notif_status_update, stats);
        }

        if (day_skip && i >= 0){
            day_skip_from_overworld(env.console, context);
            save_game_from_overworld(env.program_info(), env.console, context);
        }

        context.wait_for_all_requests();
        action(i);

        enter_menu_from_overworld(env.program_info(), env.console, context, -1);
       
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

void move_player_forward(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    uint8_t num_rounds, 
    std::function<void()>&& recovery_action,
    bool use_lets_go,
    bool mash_A,
    uint16_t forward_ticks, 
    uint8_t y, 
    uint16_t delay_after_forward_move, 
    uint16_t delay_after_lets_go
){

    context.wait_for_all_requests();
    for (size_t i = 0; i < num_rounds; i++){
        try{
            do_action_and_monitor_for_battles_early(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                if (!use_lets_go){
                    // pbf_move_left_joystick(context, 128, y, forward_ticks, 0);
                    ssf_press_left_joystick(context, 128, 0, 0, 100, 0);

                    if (mash_A){ // mashing A and Let's go aren't compatible. you end up talking to your Let's go pokemon if you mash A.
                        pbf_mash_button(context, BUTTON_A, forward_ticks);
                    }
                }else{
                    pbf_press_button(context, BUTTON_R, 20, delay_after_lets_go);
                    pbf_move_left_joystick(context, 128, y, forward_ticks, delay_after_forward_move);    
                }
            });
        }catch (UnexpectedBattleException&){
            recovery_action();
            // run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
        }
    }

}

ImageFloatBox get_yolo_box(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    VideoOverlaySet& overlays,
    YOLOv5Detector& yolo_detector, 
    const std::string& target_label
){
    context.wait_for_all_requests();
    overlays.clear();
    const std::vector<YOLOv5Session::DetectionBox>& detected_boxes = yolo_detector.detected_boxes();
    auto snapshot = env.console.video().snapshot();
    yolo_detector.detect(snapshot);

    ImageFloatBox target_box{-1, -1, -1, -1};
    double best_score = 0;
    for (YOLOv5Session::DetectionBox detected_box : detected_boxes){
        ImageFloatBox box = detected_box.box;
        std::string label = yolo_detector.session()->label_name(detected_box.label_idx);
        double score = detected_box.score;
        std::string label_score = label + ": " + tostr_fixed(score, 2);
        if (target_label == label){
            overlays.add(COLOR_RED, box, label_score);
            
            if (score > best_score){
                target_box = box;
                best_score = score;
            }
        }else{
            overlays.add(COLOR_BLUE, box, label);
        }
        
    }

    env.console.log(std::string(target_label) + ": {" + std::to_string(target_box.x) + ", " + std::to_string(target_box.y) + ", " + std::to_string(target_box.width) + ", " + std::to_string(target_box.height) + "}");
    env.console.log("center-y: " + std::to_string(target_box.y + target_box.height/2) + "   center-x: " + std::to_string(target_box.x + target_box.width/2));

    return target_box;
}

void move_forward_until_yolo_object_above_min_size(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    YOLOv5Detector& yolo_detector, 
    const std::string& target_label,
    double min_width, double min_height,
    std::function<void()>&& recovery_action, 
    uint16_t forward_ticks, 
    uint8_t y, 
    uint16_t delay_after_forward_move, 
    uint16_t delay_after_lets_go
){
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera

    VideoOverlaySet overlays(env.console.overlay());
    bool seen_object = false;
    size_t not_detected_cont = 0;
    size_t max_not_detected = 5;
    size_t forward_move_count = 0;
    bool reached_target = false;
    bool exceed_times_not_detected = false;
    while(!reached_target && !exceed_times_not_detected){  
        forward_move_count++;
        try{
            do_action_and_monitor_for_battles_early(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                context.wait_for_all_requests();
                ImageFloatBox target_box = get_yolo_box(env, context, overlays, yolo_detector, target_label);

                bool not_found_target = target_box.x == -1;
                if (not_found_target){
                    not_detected_cont++;
                    if (not_detected_cont > max_not_detected){
                        exceed_times_not_detected = true;
                        return;      // when too many failed attempts, just assume we're too close to the target to detect it.
                    }
                    context.wait_for(1000ms); // if we can't see the object, it might just be temporarily obscured. wait one second and reattempt.
                    return;
                }else{
                    seen_object = true;
                    not_detected_cont = 0;
                }

                if (target_box.width > min_width && target_box.height > min_height){
                    reached_target = true;
                    return; // stop when the target is above a certain size. i.e. we are close enough to the target.
                }
            
                pbf_move_left_joystick(context, 128, y, forward_ticks, 0);
                // pbf_press_button(context, BUTTON_R, 20, delay_after_lets_go);
                // pbf_move_left_joystick(context, 128, y, forward_ticks, delay_after_forward_move);
            });
        }catch (UnexpectedBattleException&){
            overlays.clear();
            recovery_action();
            // run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
            // move_camera_yolo();
        }

        if (forward_move_count > 50){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "move_forward_until_yolo_object_above_min_size(): Unable to reach target object after many attempts.",
                env.console
            );
        }
    }

    if (!seen_object){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "move_forward_until_yolo_object_above_min_size(): Never detected the yolo object.",
            env.console
        );
    }
}



void move_player_until_yolo_object_detected(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    YOLOv5Detector& yolo_detector, 
    const std::string& target_label,   
    std::function<void()>&& recovery_action, 
    uint16_t max_rounds, 
    uint16_t forward_ticks, 
    uint8_t x, 
    uint8_t y, 
    uint16_t delay_after_forward_move, 
    uint16_t delay_after_lets_go
){
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera

    VideoOverlaySet overlays(env.console.overlay());
    bool found_target = false;
    size_t round_num = 0;
    while(!found_target){
        try{
            do_action_and_monitor_for_battles_early(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                context.wait_for_all_requests();
                ImageFloatBox target_box = get_yolo_box(env, context, overlays, yolo_detector, target_label);
                found_target = target_box.x != -1;
                if (found_target){
                    return;
                }

                

                pbf_move_left_joystick(context, x, y, forward_ticks, 0);
                // pbf_press_button(context, BUTTON_R, 20, delay_after_lets_go);
                // pbf_move_left_joystick(context, 128, y, forward_ticks, delay_after_forward_move);
            });
            
        }catch (UnexpectedBattleException&){
            overlays.clear();
            recovery_action();
            // run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
            // move_camera_yolo();
        }


        round_num++;
        if (round_num > max_rounds){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "move_player_until_yolo_object_detected(): Unable to detect target object.",
                env.console
            );  
        }
    }
}

void move_forward_until_yolo_object_not_detected(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    YOLOv5Detector& yolo_detector, 
    const std::string& target_label,   
    size_t times_not_seen_threshold,
    std::function<void()>&& recovery_action, 
    uint16_t forward_ticks, 
    uint8_t y, 
    uint16_t delay_after_forward_move, 
    uint16_t delay_after_lets_go
){
    VideoOverlaySet overlays(env.console.overlay());
    bool target_visible = true;
    size_t max_rounds = 50;
    size_t times_not_seen = 0;
    size_t round_num = 0;
    while(times_not_seen < times_not_seen_threshold){
    try{
        do_action_and_monitor_for_battles_early(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            context.wait_for_all_requests();
            ImageFloatBox target_box = get_yolo_box(env, context, overlays, yolo_detector, target_label);
            target_visible = target_box.x != -1;
            if (!target_visible){  // stop when target not visible
                times_not_seen++;
                if (times_not_seen >= times_not_seen_threshold){
                    return;
                }
            }
            
            pbf_move_left_joystick(context, 128, y, forward_ticks, 0);
            // pbf_press_button(context, BUTTON_R, 20, delay_after_lets_go);
            // pbf_move_left_joystick(context, 128, y, forward_ticks, delay_after_forward_move);
        });
        
    }catch (UnexpectedBattleException&){
        overlays.clear();
        recovery_action();
        // run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
        // move_camera_yolo();
    }
    round_num++;

    if (round_num > max_rounds){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "move_forward_until_yolo_object_not_detected(): Unable to walk away from target object.",
            env.console
        );  
    }
    }
}


void move_camera_yolo(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    CameraAxis axis,
    YOLOv5Detector& yolo_detector, 
    const std::string& target_label,
    double target_line,
    std::function<void()>&& recovery_action
){
    VideoOverlaySet overlays(env.console.overlay());
    bool seen_object = false;
    size_t max_attempts = 20;
    size_t not_detected_count = 0;
    size_t max_not_detected = 5;
    bool reached_target_line = false;
    bool exceed_max_not_detected = false;
    for (size_t i = 0; i < max_attempts; i++){
    try{
        do_action_and_monitor_for_battles_early(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            context.wait_for_all_requests();
            ImageFloatBox target_box = get_yolo_box(env, context, overlays, yolo_detector, target_label);

            bool not_found_target = target_box.x == -1;
            if (not_found_target){
                not_detected_count++;
                if (not_detected_count > max_not_detected){
                    exceed_max_not_detected = true;
                    return;      // when too many failed attempts, just assume we're too close to the target to detect it.
                }
                context.wait_for(1000ms); // if we can't see the object, it might just be temporarily obscured. wait one second and reattempt.
                return;
            }else{
                not_detected_count = 0;
                seen_object = true;
            }
       
            
            double diff;
            switch(axis){
            case CameraAxis::X:{
                double object_x_pos = target_box.x + target_box.width/2;
                diff =  target_line - object_x_pos;
                break;
            }
            case CameraAxis::Y:{
                double object_y_pos = target_box.y + target_box.height/2;
                diff =  target_line - object_y_pos;
                break;
            }
            default:
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "move_camera_yolo: Unknown CameraAxis enum.");  
            }
            env.console.log("target_line: " + std::to_string(target_line));
            env.console.log("diff: " + std::to_string(diff));
            if (std::abs(diff) < 0.01){
                reached_target_line = true;
                return;    // close enough to target_line. stop.
            }

            
            double duration_scale_factor;
            double push_magnitude_scale_factor;
            switch(axis){
            case CameraAxis::X:
                duration_scale_factor = 250 / std::sqrt(std::abs(diff));
                if (std::abs(diff) < 0.05){
                    duration_scale_factor /= 2;
                }
                push_magnitude_scale_factor = 60 / std::sqrt(std::abs(diff));
                break;
            case CameraAxis::Y:
                duration_scale_factor = 50 / std::sqrt(std::abs(diff));
                if (std::abs(diff) < 0.1){
                    duration_scale_factor *= 0.5;
                }
                push_magnitude_scale_factor = 60 / std::sqrt(std::abs(diff));
                break;
            
            default:
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "move_camera_yolo: Unknown CameraAxis enum.");  
            }

            uint16_t push_duration = std::max(uint16_t(std::abs(diff * duration_scale_factor)), uint16_t(8));
            int16_t push_direction = (diff > 0) ? -1 : 1;
            double push_magnitude = std::max(double(std::abs(diff * push_magnitude_scale_factor)), double(15)); 
            uint8_t axis_push = uint8_t(std::max(std::min(int(128 + (push_direction * push_magnitude)), 255), 0));

            // env.console.log("object_x: {" + std::to_string(target_box.x) + ", " + std::to_string(target_box.y) + ", " + std::to_string(target_box.width) + ", " + std::to_string(target_box.height) + "}");
            // env.console.log("object_x_pos: " + std::to_string(object_x_pos));
            env.console.log("axis push: " + std::to_string(axis_push) + ", push duration: " +  std::to_string(push_duration));
            switch(axis){
            case CameraAxis::X:{
                pbf_move_right_joystick(context, axis_push, 128, push_duration, 0);
                break;
            }
            case CameraAxis::Y:{
                pbf_move_right_joystick(context, 128, axis_push, push_duration, 0);
                break;
            }
            default:
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "move_camera_yolo: Unknown CameraAxis enum.");  
            }
        });

        if(reached_target_line || exceed_max_not_detected){
            break;
        }
    
    }catch (UnexpectedBattleException&){
        overlays.clear();
        recovery_action();
        // run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
    }
    }

    if (!seen_object){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "move_camera_yolo(): Never detected the yolo object.",
            env.console
        );
    }
}

bool move_player_to_realign_via_yolo(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    YOLOv5Detector& yolo_detector, 
    const std::string& target_label,
    double x_target
){

    VideoOverlaySet overlays(env.console.overlay());
    bool seen_object = false;
    size_t max_attempts = 10;
    size_t not_detected_count = 0;
    size_t max_not_detected = 5;
    bool reached_target_line = false;
    bool exceed_max_not_detected = false;
    for (size_t i = 0; i < max_attempts; i++){
    try{
        do_action_and_monitor_for_battles_early(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            context.wait_for_all_requests();
            ImageFloatBox target_box = get_yolo_box(env, context, overlays, yolo_detector, target_label);

            bool not_found_target = target_box.x == -1;
            if (not_found_target){
                not_detected_count++;
                if (not_detected_count > max_not_detected){
                    exceed_max_not_detected = true;
                    return;      // when too many failed attempts, just assume we're too close to the target to detect it.
                }
                context.wait_for(1000ms); // if we can't see the object, it might just be temporarily obscured. wait one second and reattempt.
                return;
            }else{
                seen_object = true;
                not_detected_count = 0;
            }
       
            double object_x_pos = target_box.x + target_box.width/2;
            double diff =  x_target - object_x_pos;

            env.console.log("x_target: " + std::to_string(x_target));
            env.console.log("diff: " + std::to_string(diff));
            if (std::abs(diff) < 0.05){
                reached_target_line = true;
                return;    // close enough to target_line. stop.
            }
            
            double duration_scale_factor = 500 / std::sqrt(std::abs(diff));
            // if (std::abs(diff) < 0.05){
            //     duration_scale_factor /= 2;
            // }
            double push_magnitude_scale_factor = 60 / std::sqrt(std::abs(diff));

            uint16_t push_duration = std::max(uint16_t(std::abs(diff * duration_scale_factor)), uint16_t(8));
            int16_t push_direction = (diff > 0) ? -1 : 1;
            double push_magnitude = std::max(double(std::abs(diff * push_magnitude_scale_factor)), double(15)); 
            uint8_t x_push = uint8_t(std::max(std::min(int(128 + (push_direction * push_magnitude)), 255), 0));

            // env.console.log("object_x: {" + std::to_string(target_box.x) + ", " + std::to_string(target_box.y) + ", " + std::to_string(target_box.width) + ", " + std::to_string(target_box.height) + "}");
            // env.console.log("object_x_pos: " + std::to_string(object_x_pos));
            env.console.log("x push: " + std::to_string(x_push) + ", push duration: " +  std::to_string(push_duration));
            if (i == 0){
                pbf_move_left_joystick(context, x_push, 128, 10, 50);
                pbf_press_button(context, BUTTON_R, 20, 105);
            }
            
            pbf_move_left_joystick(context, x_push, 128, push_duration, 0);
            
        });

        if (exceed_max_not_detected){
            return false;
        }

        if(reached_target_line){
            return true;
        }
    
    }catch (UnexpectedBattleException&){
        overlays.clear();
        run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
    }
    }

    if (!seen_object){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "move_player_to_realign_via_yolo(): Never detected the yolo object.",
            env.console
        );
    }

    return false;
}


void move_player_to_realign_via_yolo_with_recovery(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    YOLOv5Detector& yolo_detector, 
    const std::string& target_label,
    double x_target,
    std::function<void()>&& recovery_action
){
    bool succeed_realign_attempt_1 = move_player_to_realign_via_yolo(env, context, yolo_detector, target_label, x_target);
    if (!succeed_realign_attempt_1){
        recovery_action();
        move_player_to_realign_via_yolo(env, context, yolo_detector, target_label, x_target); // second attempt.
    }
}


void move_camera_until_yolo_object_detected(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    YOLOv5Detector& yolo_detector, 
    const std::string& target_label,
    uint8_t initial_x_move, 
    uint16_t initial_hold_ticks,
    uint16_t max_rounds
){
    VideoOverlaySet overlays(env.console.overlay());
    bool found_target = false;
    size_t round_num = 0;
    uint8_t x_move = initial_x_move > 128 ? 255 : 0;
    while(!found_target){
        try{
            do_action_and_monitor_for_battles_early(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){

                if (round_num == 0){
                    pbf_move_right_joystick(context, initial_x_move, 128, initial_hold_ticks, 50);
                }
                context.wait_for_all_requests();
                ImageFloatBox target_box = get_yolo_box(env, context, overlays, yolo_detector, target_label);
                found_target = target_box.x != -1;
                if (found_target){
                    return;
                }

                

                pbf_move_right_joystick(context, x_move, 128, 10, 50);
            });
            
        }catch (UnexpectedBattleException&){
            overlays.clear();
            run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
        }
        round_num++;
        if (round_num > max_rounds){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "move_camera_until_yolo_object_detected(): Unable to detect target object.",
                env.console
            );  
        }
    }
}



}
}
}

