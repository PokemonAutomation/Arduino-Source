/*  Auto Story Segment 01
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "VideoGames/PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "VideoGames/PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA_AutoStory_Segment_01.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

// Opening sequences
void checkpoint_00(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // Move to the Start Game button
    context.wait_for_all_requests();
    SelectionArrowWatcher begin_arrow(
        COLOR_GREEN, &env.console.overlay(), SelectionArrowType::RIGHT,
        {0.05, 0.73, 0.045, 0.12}, 100ms
    );
    const WallClock deadline = current_time() + 30s;
    
    while (wait_until(env.console, context, std::min(deadline, current_time() + 300ms), {begin_arrow}) < 0){
        if (current_time() >= deadline){
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::SEND_ERROR_REPORT,
                "Unable to select Begin on the profile confirmation screen within 30 seconds.",
                env.console
            );
        }
        pbf_press_dpad(context, DPAD_DOWN, 100ms, 300ms);
        context.wait_for_all_requests();
    }

    // Opening cutscene
    pbf_press_button(context, BUTTON_A, 100ms, 15000ms);
    press_button_until_dialog(env.console, context, BUTTON_PLUS, 2000ms, 100ms, 60s);
    pbf_press_button(context, BUTTON_A, 100ms, 100ms);

    // Wait out the train as it can false positive on the button detector
    pbf_wait(context, 13000ms);
    // Keep the similar left-stick icon in the movement row out of the search.
    const ImageFloatBox camera_hint_box{0.35, 0.902, 0.30, 0.028};
    mash_button_until_right_stick(
        env.console, context, BUTTON_B,
        camera_hint_box, 60s
    );

    // Change text speed to fast (consider turn off rumble too if it causes issues in bluetooth traffic)
    pbf_press_button(context, BUTTON_X, 100ms, 2600ms);
    pbf_press_button(context, BUTTON_A, 100ms, 600ms);
    pbf_press_button(context, BUTTON_L, 100ms, 600ms);
    pbf_press_dpad(context, DPAD_UP, 100ms, 300ms);
    pbf_press_button(context, BUTTON_A, 100ms, 1100ms);
    pbf_press_dpad(context, DPAD_DOWN, 100ms, 300ms);
    pbf_press_dpad(context, DPAD_RIGHT, 100ms, 300ms);
    mash_button_until_right_stick(
        env.console, context, BUTTON_B,
        camera_hint_box, 60s
    );

    // Exit station and mash through the NPC introduction cutscene
    pbf_move_left_joystick(context, {0.25, 1}, 4000ms, 100ms);
    pbf_move_left_joystick(context, {-0.8, 0.7}, 500ms, 100ms);
    pbf_move_left_joystick(context, {0.95, 0.35}, 5000ms, 100ms);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY,
        0.95, -0.4, 60s
    );
    clear_dialog(env.console, context, ClearDialogMode::STOP_BLACK_SCREEN);
    press_button_until_dialog(env.console, context, BUTTON_PLUS, 2000ms, 100ms, 60s);
    clear_dialog(env.console, context, ClearDialogMode::STOP_BLACK_SCREEN);
    pbf_wait(context, 2000ms);
}

// Starter selection and battle tutorial
void checkpoint_01(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    BattleAIOption& battle_ai, const JoystickPosition& starter_direction
){
    // Chase Pancham
    pbf_move_left_joystick(context, {-0.3, 1}, 5500ms, 100ms);
    pbf_move_left_joystick(context, {-0.95, -0.15}, 4000ms, 100ms);
    pbf_move_left_joystick(context, {0.07, 1}, 6000ms, 100ms);
    // Small pause to avoid colliding with the starters
    pbf_wait(context, 800ms);
    pbf_move_left_joystick(context, {0.07, 1}, 4000ms, 100ms);
    pbf_move_left_joystick(context, {-1, 0.2}, 2000ms, 100ms);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY,
        -0.9, 0.5, 60s
    );
    clear_dialog(env.console, context, ClearDialogMode::STOP_BLACK_SCREEN);
    pbf_wait(context, 2000ms);

    // Starter selection and tutorial battle
    pbf_move_left_joystick(context, starter_direction, 500ms, 0ms);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_BLACK_SCREEN, NavigationMovementMode::DIRECTIONAL_SPAM_A,
        starter_direction.x, starter_direction.y, 60s
    );
    clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE);
    run_battle_until_condition(env, context, battle_ai, BattleStopCondition::STOP_BLACK_SCREEN);
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD);
}

// First battle zone and Hotel Z introduction
void checkpoint_02(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    BattleAIOption& battle_ai
){
    // NPC battle
    pbf_move_left_joystick(context, {0, 1}, 5000ms, 100ms);
    pbf_move_left_joystick(context, {1, 0.2}, 4000ms, 100ms);
    pbf_move_left_joystick(context, {0.6, -0.8}, 800ms, 100ms);
    pbf_move_left_joystick(context, {1, 0}, 3000ms, 100ms);
    pbf_move_left_joystick(context, {1, 0.25}, 2000ms, 100ms);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_SPAM_A,
        0, -1, 60s
    );
    run_battle_until_condition(env, context, battle_ai, BattleStopCondition::STOP_OVERWORLD);

    // Floette cutscene and exiting the battle zone
    pbf_move_left_joystick(context, {-0.4, 0.95}, 3000ms, 100ms);
    pbf_move_left_joystick(context, {-1, -0.15}, 4500ms, 100ms);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_BLACK_SCREEN, NavigationMovementMode::DIRECTIONAL_ONLY,
        0.5, 0.85, 60s
    );
    press_button_until_dialog(env.console, context, BUTTON_PLUS, 2000ms, 100ms, 60s);
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_BUTTON, NavigationMovementMode::DIRECTIONAL_ONLY,
        0, +1, 30s, true, ButtonType::ButtonA
    );
    pbf_press_button(context, BUTTON_A, 100ms, 500ms);
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD);

    // Head to Hotel Z
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_BLACK_SCREEN, NavigationMovementMode::DIRECTIONAL_ONLY,
        +1, 0, 60s
    );
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD);
    pbf_move_left_joystick(context, {0, 1}, 4000ms, 100ms);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_BUTTON, NavigationMovementMode::DIRECTIONAL_ONLY,
        0, +1, 30s, true, ButtonType::ButtonA
    );
    pbf_press_button(context, BUTTON_A, 100ms, 500ms);
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD);

    // Enter hotel room
    pbf_move_left_joystick(context, {-0.13, 1}, 4000ms, 100ms);
    pbf_move_left_joystick(context, {-1, -0.1}, 300ms, 100ms);
    mash_button_until_door_entry(env.console, context);
    pbf_move_left_joystick(context, {0, 1}, 1800ms, 100ms);
    pbf_mash_button(context, BUTTON_A, 1s);
    mash_button_until_door_entry(env.console, context);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_BUTTON, NavigationMovementMode::DIRECTIONAL_ONLY,
        -0.2, 0.95, 60s, false, ButtonType::ButtonA
    );
    pbf_mash_button(context, BUTTON_A, 1s);
    clear_dialog(env.console, context, ClearDialogMode::STOP_BLACK_SCREEN);
    pbf_wait(context, 2000ms);

    // Approach the bed, then leave for the rooftop conversation
    pbf_move_left_joystick(context, {0.8, 0.8}, 8000ms, 100ms);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_BUTTON, NavigationMovementMode::DIRECTIONAL_ONLY,
        0, 1, 60s, false, ButtonType::ButtonA
    );
    pbf_mash_button(context, BUTTON_A, 2s);
    mash_button_until_door_entry(env.console, context);
    pbf_move_left_joystick(context, {0, 1}, 800ms, 100ms);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_BUTTON, NavigationMovementMode::DIRECTIONAL_ONLY,
        -0.2, 0.8, 60s, false, ButtonType::ButtonA
    );
    pbf_mash_button(context, BUTTON_A, 2s);
    mash_button_until_door_entry(env.console, context);
}

// First rival battle and Pokemon Center introduction
void checkpoint_03(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    BattleAIOption& battle_ai
){
    // Z-A Royale introduction
    pbf_move_left_joystick(context, {0, 1}, 800ms, 100ms);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_BUTTON, NavigationMovementMode::DIRECTIONAL_ONLY,
        -0.8, 0.8, 60s, false, ButtonType::ButtonA
    );
    clear_dialog(env.console, context, ClearDialogMode::STOP_BLACK_SCREEN);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_BUTTON, NavigationMovementMode::DIRECTIONAL_ONLY,
        -0.3, 0.95, 60s, true, ButtonType::ButtonA
    );
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, BUTTON_A, 6min);

    // Rival battle
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_BUTTON, NavigationMovementMode::DIRECTIONAL_ONLY,
        0, 1, 60s, true, ButtonType::ButtonA
    );
    pbf_press_button(context, BUTTON_A, 100ms, 500ms);
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_BUTTON, NavigationMovementMode::DIRECTIONAL_ONLY,
        0, 1, 60s, false, ButtonType::ButtonA
    );
    clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE);
    run_battle_until_condition(env, context, battle_ai, BattleStopCondition::STOP_OVERWORLD);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY,
        0, 1, 60s, true
    );
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD);

    // Climb the ladder and finish the movement tutorial
    pbf_move_left_joystick(context, {0.3, 1}, 9000ms, 100ms);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_BUTTON, NavigationMovementMode::DIRECTIONAL_ONLY,
        -1, 1, 60s, false, ButtonType::ButtonA
    );
    pbf_mash_button(context, BUTTON_A, 1s);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY,
        0, 1, 60s
    );
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY,
        0, 1, 60s, true
    );
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD);

    // Move to the Pokemon Center and finish the tutorial
    pbf_move_left_joystick(context, {0, 1}, 8000ms, 100ms);
    pbf_move_left_joystick(context, {1, 0}, 3000ms, 100ms);
    pbf_move_left_joystick(context, {0, 1}, 3400ms, 100ms);
    pbf_move_left_joystick(context, {-1, 0}, 6000ms, 100ms);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_BUTTON, NavigationMovementMode::DIRECTIONAL_ONLY,
        0, 1, 60s, false, ButtonType::ButtonA
    );
    clear_dialog(env.console, context, ClearDialogMode::STOP_WHITE_SCREEN);
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, BUTTON_B);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY,
        0, 1, 60s, true
    );
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD);
}

// Catching tutorials and Mable's Research introduction
void checkpoint_04(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    BattleAIOption& battle_ai
){
    // Enter Wild Zone 1
    pbf_move_left_joystick(context, {0, 1}, 4000ms, 100ms);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_BUTTON, NavigationMovementMode::DIRECTIONAL_ONLY,
        0, 1, 60s, true, ButtonType::ButtonA
    );
    pbf_mash_button(context, BUTTON_A, 2s);
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD);

    // First Fletchling tutorial, always fails
    run_catch(context);
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD);

    // Second Fletchling tutorial, always succeeds
    run_catch(context);
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD);

    // Bunnelby tutorial, faint first then catch
    pbf_press_button(context, BUTTON_UP, 500ms, 2s);
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);
    run_wild_battle_until_condition(env, context, battle_ai, WildBattleStopCondition::STOP_DIALOG);
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD);
    run_catch(context);
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD);

    // Catch Scatterbug
    pbf_move_left_joystick(context, {-1, 0}, 1000ms, 100ms);
    pbf_move_left_joystick(context, {0, 1}, 5500ms, 100ms);
    ssf_press_right_joystick(context, {1, 0}, 0ms, 300ms, 0ms);
    pbf_move_left_joystick(context, {0, 1}, 300ms, 100ms);
    pbf_move_left_joystick(context, {0, 1}, 400ms, 100ms);
    run_catch(context, 9000ms);

    // Use the news booth as a camera landmark, then pass it on the left toward Weedle
    pbf_move_left_joystick(context, {0, 1}, 500ms, 100ms);
    align_camera_left_of_booth(env.console, context);
    pbf_move_left_joystick(context, {0, 1}, 5000ms, 100ms);

    // Attempt 5 Weedle catches, rotating camera angle and checking party count
    bool caught = false;
    for (size_t attempt = 0; attempt < 5; attempt++){
        context.throw_if_cancelled();
        env.log("Weedle catch attempt " + std::to_string(attempt + 1) + " of 5.");
        if (attempt > 0){
            // Each retry adds another small rightward camera turn, without moving.
            pbf_move_right_joystick(context, {0.35, 0}, 1000ms, 200ms);
        }
        run_catch(context);
        if (party_has_at_least(env.console, context, 5)){
            caught = true;
            break;
        }
    }
    if (!caught){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "Fewer than five party Pokemon after five Weedle catch attempts.",
            env.console
        );
    }

    // Adjust direction based on the event marker and exit Wild Zone 1
    center_event_marker(env.console, context);
    pbf_move_left_joystick(context, {0.2, 1}, 3000ms, 100ms);
    center_event_marker(env.console, context);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_BUTTON, NavigationMovementMode::DIRECTIONAL_ONLY,
        0, 1, 60s, true, ButtonType::ButtonA
    );
    pbf_mash_button(context, BUTTON_A, 2s);

    // Adjust direction based on the event marker and enter research lab
    center_event_marker(env.console, context);
    pbf_move_left_joystick(context, {0.4, 1}, 6000ms, 100ms);
    center_event_marker(env.console, context);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY,
        0, 1, 60s, true
    );
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_BUTTON, NavigationMovementMode::DIRECTIONAL_ONLY,
        0, 1, 60s, true, ButtonType::ButtonA
    );
    pbf_mash_button(context, BUTTON_A, 2s);
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD);

    // Enter elevator and go to the 3rd floor
    center_event_marker(env.console, context);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_BUTTON, NavigationMovementMode::DIRECTIONAL_ONLY,
        0, 1, 60s, false, ButtonType::ButtonA
    );
    clear_dialog(env.console, context, ClearDialogMode::STOP_CHOICES);
    pbf_press_dpad(context, DPAD_DOWN, 100ms, 300ms);
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD);

    // Marble's Research introduction
    pbf_move_left_joystick(context, {-0.6, 0.9}, 1500ms, 100ms);
    overworld_navigation(
        env.console, context,
        NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY,
        1, -0.2, 60s
    );
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, BUTTON_B);

    press_button_until_dialog(env.console, context, BUTTON_X);
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, BUTTON_B);
}

std::string AutoStory_Segment_01::name() const{
    return "01: Tutorial (Until unlocking Mystery Gift)";
}

std::string AutoStory_Segment_01::start_text() const{
    return "Start: Profile confirmation screen after choosing the game language, player appearance, and player name";
}

std::string AutoStory_Segment_01::end_text() const{
    return "End: Introduction to Mable's Research and unlocking Mystery Gift etc.";
}

void AutoStory_Segment_01::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment = 1;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);
    env.log(start_text());

    AutoStory_Checkpoint_00().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_01().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_02().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_03().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_04().run_checkpoint(env, context, options, stats);

    env.log(end_text());
    env.console.log("End Segment " + name(), COLOR_GREEN);
}

std::string AutoStory_Checkpoint_00::name() const{
    return "00: Opening sequences";
}

std::string AutoStory_Checkpoint_00::start_text() const{
    return "Profile confirmation screen after choosing the language, appearance, and name";
}

std::string AutoStory_Checkpoint_00::end_text() const{
    return "Outside the station before chasing Pancham";
}

void AutoStory_Checkpoint_00::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions,
    AutoStoryStats& stats
) const{
    stats.m_segment = 1;
    stats.m_checkpoint = 0;
    env.update_stats();
    context.wait_for_all_requests();
    env.log("Checkpoint " + name());
    env.log("Start: " + start_text());
    checkpoint_00(env, context);
    context.wait_for_all_requests();
}

std::string AutoStory_Checkpoint_01::name() const{
    return "01: Starter selection and battle tutorial";
}

std::string AutoStory_Checkpoint_01::start_text() const{
    return "Outside the station before chasing Pancham";
}

std::string AutoStory_Checkpoint_01::end_text() const{
    return "Finished first battle tutorial, battle zone started";
}

void AutoStory_Checkpoint_01::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment = 1;
    stats.m_checkpoint = 1;
    env.update_stats();
    context.wait_for_all_requests();
    env.log("Checkpoint " + name());
    env.log("Start: " + start_text());
    JoystickPosition starter_direction;
    switch (options.starter_choice){
    case StarterChoice::CHIKORITA:
        starter_direction = {-0.9, +0.4};
        break;
    case StarterChoice::TOTODILE:
        starter_direction = {-0.9, +0.75};
        break;
    case StarterChoice::TEPIG:
        starter_direction = {-0.6, +0.9};
        break;
    }
    checkpoint_01(env, context, options.battle_ai, starter_direction);
    context.wait_for_all_requests();
}

std::string AutoStory_Checkpoint_02::name() const{
    return "02: First battle zone and Hotel Z introduction";
}

std::string AutoStory_Checkpoint_02::start_text() const{
    return "Finished first battle tutorial, battle zone started";
}

std::string AutoStory_Checkpoint_02::end_text() const{
    return "Hotel Z night sequence finished, next morning";
}

void AutoStory_Checkpoint_02::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment = 1;
    stats.m_checkpoint = 2;
    env.update_stats();
    context.wait_for_all_requests();
    env.log("Checkpoint " + name());
    env.log("Start: " + start_text());
    checkpoint_02(env, context, options.battle_ai);
    context.wait_for_all_requests();
}

std::string AutoStory_Checkpoint_03::name() const{
    return "03: First rival battle and Pokemon Center introduction";
}

std::string AutoStory_Checkpoint_03::start_text() const{
    return "Hotel Z night sequence finished, next morning";
}

std::string AutoStory_Checkpoint_03::end_text() const{
    return "Pokemon Center introduction completed, before entering Wild Zone 1";
}

void AutoStory_Checkpoint_03::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment = 1;
    stats.m_checkpoint = 3;
    env.update_stats();
    context.wait_for_all_requests();
    env.log("Checkpoint " + name());
    env.log("Start: " + start_text());
    checkpoint_03(env, context, options.battle_ai);
    context.wait_for_all_requests();
}

std::string AutoStory_Checkpoint_04::name() const{
    return "04: Catching tutorials and Mable's Research introduction";
}

std::string AutoStory_Checkpoint_04::start_text() const{
    return "Pokemon Center introduction completed, before entering Wild Zone 1";
}

std::string AutoStory_Checkpoint_04::end_text() const{
    return "Mable's Research introduction completed, inside research lab";
}

void AutoStory_Checkpoint_04::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment = 1;
    stats.m_checkpoint = 4;
    env.update_stats();
    context.wait_for_all_requests();
    env.log("Checkpoint " + name());
    env.log("Start: " + start_text());
    checkpoint_04(env, context, options.battle_ai);
    context.wait_for_all_requests();
}

}
}
}
