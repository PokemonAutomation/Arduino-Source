/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Programs/Battles/PokemonSV_SinglesBattler.h"
#include "PokemonSV/Programs/Battles/PokemonSV_Battles.h"

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_33.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_33::name() const{
    return "33: Clavell battle, Elite Four";
}

std::string AutoStory_Segment_33::start_text() const{
    return "Start: Beat Alfornada Gym (Psychic). At Alfornada Pokecenter.";
}

std::string AutoStory_Segment_33::end_text() const{
    return "End: Beat Clavell. Beat Elite Four. At Pokemon League Pokecenter.";
}

void AutoStory_Segment_33::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_85().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_86().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_87().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_88().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_89().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}


std::string AutoStory_Checkpoint_85::name() const{ return "085 - " + AutoStory_Segment_33().name(); }
std::string AutoStory_Checkpoint_85::start_text() const{ return "Beat Alfornada gym challenge. Beat Alfornada gym. At Alfronada Pokecenter.";}
std::string AutoStory_Checkpoint_85::end_text() const{ return "Beat Clavell. At Academy fly point.";}
void AutoStory_Checkpoint_85::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_85(env, context, options.notif_status_update, stats);
}


std::string AutoStory_Checkpoint_86::name() const{ return "086 - " + AutoStory_Segment_33().name(); }
std::string AutoStory_Checkpoint_86::start_text() const{ return AutoStory_Checkpoint_85().end_text();}
std::string AutoStory_Checkpoint_86::end_text() const{ return "At Pokemon League entrance.";}
void AutoStory_Checkpoint_86::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_86(env, context, options.notif_status_update, stats);
}


std::string AutoStory_Checkpoint_87::name() const{ return "087 - " + AutoStory_Segment_33().name(); }
std::string AutoStory_Checkpoint_87::start_text() const{ return AutoStory_Checkpoint_86().end_text();}
std::string AutoStory_Checkpoint_87::end_text() const{ return "Finished the Entrance quiz. Standing in front of Rika.";}
void AutoStory_Checkpoint_87::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_87(env, context, options.notif_status_update, stats, options.language, options.starter_choice);
}


std::string AutoStory_Checkpoint_88::name() const{ return "088 - " + AutoStory_Segment_33().name(); }
std::string AutoStory_Checkpoint_88::start_text() const{ return AutoStory_Checkpoint_87().end_text();}
std::string AutoStory_Checkpoint_88::end_text() const{ return "Beat Elite Four.";}
void AutoStory_Checkpoint_88::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_88(env, context, options.notif_status_update, stats);
}


std::string AutoStory_Checkpoint_89::name() const{ return "089 - " + AutoStory_Segment_33().name(); }
std::string AutoStory_Checkpoint_89::start_text() const{ return AutoStory_Checkpoint_88().end_text();}
std::string AutoStory_Checkpoint_89::end_text() const{ return "Beat Geeta. At Pokemon League Pokecenter.";}
void AutoStory_Checkpoint_89::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_89(env, context, options.notif_status_update, stats);
}




void checkpoint_85(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 255, 50, 2560ms}, FlyPoint::FAST_TRAVEL);

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 128, 0, 640ms);
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 30000ms);

        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::PROMPT_DIALOG, CallbackEnum::BATTLE, CallbackEnum:: DIALOG_ARROW});

        env.console.log("Battle Clavell.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A);

        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_IN, 0, 0, 0ms}, FlyPoint::FAST_TRAVEL);
    });  
}


void checkpoint_86(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 128, 400ms);
        DirectionDetector direction;
        // minimap should be clear of Pokemon within Mesagoza
        direction.change_direction(env.program_info(), env.console, context, 1.222127);
        pbf_move_left_joystick(context, {0, +1}, 8800ms, 400ms);

        get_on_ride(env.program_info(), env.console, context);
        direction.change_direction(env.program_info(), env.console, context, 1.484555);

        pbf_move_left_joystick(context, {0, +1}, 1506ms, 0ms);
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, {0, +1}, {0, 0}, 703ms);
        pbf_move_left_joystick(context, {0, +1}, 233ms, 0ms);
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, {0, +1}, {0, 0}, 5098ms);
        pbf_move_left_joystick(context, {0, +1}, 1000ms, 0ms);

        wait_for_overworld(env.program_info(), env.console, context);

        // marker 1    {0.429688, 0.299074}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 0, 0ms}, 
            FlyPoint::POKECENTER, 
            {0.429688, 0.299074}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 30, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );


        // marker 2    {0.482812, 0.378704}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 0, 0, 400ms},
            FlyPoint::POKECENTER, 
            {0.482812, 0.378704}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 40, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );


        // marker 3    {0.638021, 0.676852}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 0, 400ms},
            FlyPoint::POKECENTER, 
            {0.638021, 0.676852}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 60, 30, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD});
        mash_button_till_overworld(env.console, context, BUTTON_A);

    });   
}

void checkpoint_87(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats, Language language, StarterChoice starter_choice){
    GameTitle game_title = GameTitle::UNKNOWN;
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        if (game_title == GameTitle::UNKNOWN){
            game_title = get_game_title(env, context);
            press_Bs_to_back_to_overworld(env.program_info(), env.console, context);
        }

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);

        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                walk_forward_while_clear_front_path(env.program_info(), env.console, context, 800ms);
                walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A);
            }
        );
        // talk to door man
        // clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD, CallbackEnum::PROMPT_DIALOG});
        mash_button_till_overworld(env.console, context, BUTTON_A);
        
        do_action_and_monitor_for_overworld(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            // move left to sit down
            pbf_move_left_joystick(context, {-1, 0}, 800ms, 400ms);

            // talk to Rika 1. Choose first option. Walked here.
            clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 60, {CallbackEnum::PROMPT_DIALOG});
            pbf_mash_button(context, BUTTON_A, 1000ms);
            
            // talk to Rika 2. Choose option based on your game title
            clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 60, {CallbackEnum::PROMPT_DIALOG});

            switch(game_title){
            case GameTitle::SCARLET:
                pbf_press_dpad(context, DPAD_DOWN, 104ms, 160ms);
                break;
            case GameTitle::VIOLET:
                pbf_press_dpad(context, DPAD_DOWN, 104ms, 160ms);
                pbf_press_dpad(context, DPAD_DOWN, 104ms, 160ms);
                break;
            default:
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "We don't know what game we are playing. We should know at this point.");
                break;
            }
            pbf_mash_button(context, BUTTON_A, 1000ms);
            
            // talk to Rika 3. Came to become a Champion
            clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 60, {CallbackEnum::PROMPT_DIALOG});
            pbf_press_dpad(context, DPAD_DOWN, 104ms, 160ms);
            pbf_mash_button(context, BUTTON_A, 1000ms);

            // talk to Rika 4. Become stronger
            clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 60, {CallbackEnum::PROMPT_DIALOG});
            pbf_mash_button(context, BUTTON_A, 1000ms);

            // talk to Rika 5. Difficult gym: Alfornada
            clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 60, {CallbackEnum::PROMPT_DIALOG});
            pbf_press_dpad(context, DPAD_UP, 104ms, 160ms);
            pbf_mash_button(context, BUTTON_A, 1000ms);

            // talk to Rika 6. Difficult gym leader name: Tulip
            clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 60, {CallbackEnum::PROMPT_DIALOG});
            pbf_press_dpad(context, DPAD_UP, 104ms, 160ms);
            pbf_mash_button(context, BUTTON_A, 1000ms);

            // talk to Rika 7. Difficult gym leader type: Psychic
            clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 60, {CallbackEnum::PROMPT_DIALOG});
            pbf_press_dpad(context, DPAD_UP, 104ms, 160ms);
            pbf_press_dpad(context, DPAD_UP, 104ms, 160ms);
            pbf_mash_button(context, BUTTON_A, 1000ms);

            // talk to Rika 8. Starter pokemon: Grass/Fire/Water
            clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 60, {CallbackEnum::PROMPT_DIALOG});
            switch(starter_choice){
            case StarterChoice::SPRIGATITO:
                break;
            case StarterChoice::FUECOCO:
                pbf_press_dpad(context, DPAD_DOWN, 104ms, 160ms);
                break;
            case StarterChoice::QUAXLY:
                pbf_press_dpad(context, DPAD_DOWN, 104ms, 160ms);
                pbf_press_dpad(context, DPAD_DOWN, 104ms, 160ms);
                break;
            default:
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid starter pokemon type. This shouldn't happen.");
                break;
            }
            size_t num_extra_clicks = attempt_number % 3;  // we add extra clicks when attempt_number > 0, to account for the fact that the user might not be enterint the correct starter.
            for (size_t i = 0; i < num_extra_clicks; i++){
                pbf_press_dpad(context, DPAD_DOWN, 104ms, 160ms);
            }

            pbf_mash_button(context, BUTTON_A, 1000ms);

            // talk to Rika 9. Become stronger
            // talk to Rika 10. do you like pokemon
            clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 20, {CallbackEnum::PROMPT_DIALOG});

        });

        pbf_move_left_joystick(context, {0, +1}, 800ms, 400ms); // stand up


    }); 
}


void checkpoint_88(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        // standing in front of Rika
        // now done talking to Rika. walk around Rika's desk.

        pbf_move_left_joystick(context, {-1, 0}, 400ms, 400ms); // go left
        pbf_move_left_joystick(context, {0, +1}, 1600ms, 400ms); // straight
        pbf_move_left_joystick(context, {+1, 0}, 400ms, 400ms); // right
        
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 60000ms);
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW});


        env.console.log("Battle Elite Four 1.");
        SinglesMoveEntry move1{SinglesMoveType::Move1, false};  // Moonblast
        SinglesMoveEntry move2{SinglesMoveType::Move2, false}; // Mystical Fire
        SinglesMoveEntry move4{SinglesMoveType::Move4, false}; // Misty Terrain
        std::vector<SinglesMoveEntry> move_table1 = {move1, move4, move1};
        bool terastallized = false;
        // use Moonblast to KO the Whiscash. Then setup Misty Terrain on the second pokemon, Camerupt.
        // Then spam Moonblast to finish off the rest of the team.
        // We setup Misty Terrain to prevent getting poisoned by the Donphan's Poison Jab, since it survives with Sturdy.
        // Getting poisoned will cause us to fail the Elite Four since we have no easy way to heal off poison.
        // We don't setup Misty Terrain on the Whiscash since Muddy Water can lower our accuracy.
        bool is_won = run_pokemon(env.console, context, move_table1, true, terastallized);
        if (!is_won){// throw exception if we lose
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to beat the Ground trainer. Reset.",
                env.console
            );
        }

        // finished battle
        mash_button_till_overworld(env.console, context, BUTTON_A);

        // heal
        auto_heal_from_menu_or_overworld(env.program_info(), env.console, context, 0, true);

        // engage next battle
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 60000ms);
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW, CallbackEnum::PROMPT_DIALOG});

        env.console.log("Battle Elite Four 2.");  // select move 2, which should be a fire move. to battle the steel trainer
        std::vector<SinglesMoveEntry> move_table2 = {move2};
        is_won = run_pokemon(env.console, context, move_table2, true, terastallized);
        if (!is_won){// throw exception if we lose
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to beat the Steel trainer. Reset.",
                env.console
            );
        }

        // finished battle
        mash_button_till_overworld(env.console, context, BUTTON_A);

        // heal
        auto_heal_from_menu_or_overworld(env.program_info(), env.console, context, 0, true);

        // engage next battle
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 60000ms);
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW, CallbackEnum::PROMPT_DIALOG});

        env.console.log("Battle Elite Four 3.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);

        // finished battle
        mash_button_till_overworld(env.console, context, BUTTON_A);

        // engage next battle
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 60000ms);
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW, CallbackEnum::PROMPT_DIALOG});

        env.console.log("Battle Elite Four 4.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);

        // finished battle
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD});

    });     
}

void checkpoint_89(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 60000ms);
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW, CallbackEnum::PROMPT_DIALOG});

        env.console.log("Battle Geeta.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);

        // finished battle
        mash_button_till_overworld(env.console, context, BUTTON_A);

        // move to Pokecenter
        handle_unexpected_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 50, 400ms);
        });      
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 15, 12, 12, false);           // can't wrap in handle_when_stationary_in_overworld(), since we expect to be stationary when walking into the pokecenter
            

        fly_to_overlapping_flypoint(env.program_info(), env.console, context); 

    });     
}



GameTitle get_game_title(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    GameTitle game_title = GameTitle::UNKNOWN;
    enter_menu_from_overworld(env.program_info(), env.console, context, 0, MenuSide::RIGHT);
    context.wait_for_all_requests();

    VideoSnapshot screen = env.console.video().snapshot();
    ImageFloatBox box = {0.03, 0.94, 0.40, 0.04};
    ImageStats bottom_stats = image_stats(extract_box_reference(screen, box));

    if (is_solid(bottom_stats, {0.648549, 0.2861580, 0.0652928}, 0.15, 25)){
        game_title = GameTitle::SCARLET;
        env.console.log("Game title detected: Scarlet");
    } else if (is_solid(bottom_stats, {0.367816, 0.0746615, 0.5575230}, 0.15, 25)){
        game_title = GameTitle::VIOLET;
        env.console.log("Game title detected: Violet");
    }

    if (game_title == GameTitle::UNKNOWN){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unable to determine what game we are playing. The color of the bottom bar in the Pokemon Summary page doesn't match of the expected colors.");
    }

    return game_title;

}


std::string get_ride_pokemon_name(SingleSwitchProgramEnvironment& env, ProControllerContext& context, Language language){
    enter_menu_from_overworld(env.program_info(), env.console, context, -1);

    ImageFloatBox box = {0.116223, 0.895000, 0.194915, 0.051724};
    ImageViewRGB32 image = extract_box_reference(env.console.video().snapshot(), box);

    OCR::StringMatchResult ocr_result = Pokemon::PokemonNameReader::instance().read_substring(
        env.console.logger(), language, image,
        OCR::WHITE_TEXT_FILTERS()
    );
    std::multimap<double, OCR::StringMatchData> results;
    if (!ocr_result.results.empty()){
        for (const auto& result : ocr_result.results){
            results.emplace(result.first, result.second);
        }
    }

    if (results.empty()){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "AutoStory_Segment_34::checkpoint_86(): Unable to read selected item. No valid results.\n" + language_warning(language),
            env.console
        );
    }

    if (results.size() > 1){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "AutoStory_Segment_34::checkpoint_86(): Unable to read selected item. Ambiguous or multiple results.\n" + language_warning(language),
            env.console
        );
    }

    std::string ride_pokemon = results.begin()->second.token;

    // cout << ride_pokemon << endl;
    return ride_pokemon;
}






}
}
}
