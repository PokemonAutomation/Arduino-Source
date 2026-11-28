/*  Auto Story Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cmath>
#include <vector>
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "VideoGames/PokemonLZA/Inference/Map/PokemonLZA_MapDetector.h"
#include "VideoGames/PokemonLZA/Inference/Map/PokemonLZA_TutorialMapDetector.h"
#include "VideoGames/PokemonLZA/Inference/PokemonLZA_BoothDetector.h"
#include "VideoGames/PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "VideoGames/PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "VideoGames/PokemonLZA/Inference/PokemonLZA_DoorEntryDetector.h"
#include "VideoGames/PokemonLZA/Inference/PokemonLZA_EventMarkerDetector.h"
#include "VideoGames/PokemonLZA/Inference/PokemonLZA_OverworldPartySelectionDetector.h"
#include "VideoGames/PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "VideoGames/PokemonLZA/Inference/PokemonLZA_TutorialWindowDetector.h"
#include "VideoGames/PokemonLZA/Programs/PokemonLZA_TrainerBattle.h"
#include "PokemonLZA_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

void clear_dialog(
    ConsoleHandle& console, ProControllerContext& context,
    ClearDialogMode mode, Button button, Milliseconds timeout
){
    if (timeout <= 0ms){
        throw InternalProgramError(&console.logger(), PA_CURRENT_FUNCTION, "Dialog timeout must be positive.");
    }
    context.wait_for_all_requests();
    const WallClock deadline = current_time() + timeout;
    constexpr Milliseconds choice_release = 1500ms;

    OverworldPartySelectionWatcher overworld(COLOR_WHITE, &console.overlay());
    BlackScreenOverWatcher black_screen_over(COLOR_BLUE);
    DetectorToFinder<WhiteScreenDetector> white_screen("WhiteScreenWatcher", 100ms, COLOR_WHITE);
    auto battle_watchers = make_trainer_battle_watchers(100ms);
    SelectionArrowWatcher dialog_option(
        COLOR_GREEN, &console.overlay(), SelectionArrowType::RIGHT,
        {0.5, 0.2, 0.45, 0.6}, 100ms
    );
    TutorialPageWatcher tutorial_page(COLOR_CYAN, &console.overlay());
    TutorialWindowWatcher tutorial_window(COLOR_CYAN);
    std::vector<PeriodicInferenceCallback> callbacks;
    const char* stop_description = nullptr;
    switch (mode){
    case ClearDialogMode::STOP_OVERWORLD:
        callbacks.emplace_back(overworld);
        stop_description = "overworld";
        break;
    case ClearDialogMode::STOP_BLACK_SCREEN:
        callbacks.emplace_back(black_screen_over);
        stop_description = "end of black-screen transition";
        break;
    case ClearDialogMode::STOP_WHITE_SCREEN:
        callbacks.emplace_back(white_screen);
        stop_description = "white screen";
        break;
    case ClearDialogMode::STOP_BATTLE:
        for (auto& watcher : battle_watchers){
            callbacks.emplace_back(watcher);
        }
        stop_description = "opponent party indicators";
        break;
    case ClearDialogMode::STOP_CHOICES:
        stop_description = "dialog choices";
        break;
    default:
        throw InternalProgramError(&console.logger(), PA_CURRENT_FUNCTION, "Unknown dialog stop condition.");
    }

    const int option_index = static_cast<int>(callbacks.size());
    callbacks.emplace_back(dialog_option);
    callbacks.emplace_back(tutorial_page);
    callbacks.emplace_back(tutorial_window);
    // Keep the stop watchers alive across choices, including black-screen state
    while (current_time() < deadline){
        context.throw_if_cancelled();
        const Milliseconds remaining = std::chrono::duration_cast<Milliseconds>(deadline - current_time());
        if (remaining <= 0ms){
            break;
        }
        dialog_option.reset_state();
        tutorial_page.reset_state();
        tutorial_window.reset_state();
        int ret = run_until<ProControllerContext>(
            console, context,
            [=](ProControllerContext& context){
                if (mode == ClearDialogMode::STOP_CHOICES){
                    // Check for choices before the first press as well as between presses
                    const Milliseconds settle = std::min(choice_release, remaining);
                    pbf_wait(context, settle);
                    for (Milliseconds left = remaining - settle; left > 0ms;){
                        context.throw_if_cancelled();
                        const Milliseconds hold = std::min(100ms, left);
                        const Milliseconds release = std::min(choice_release, left - hold);
                        pbf_press_button(context, button, hold, release);
                        left -= hold + release;
                    }
                }else{
                    pbf_mash_button(context, button, remaining);
                }
            },
            callbacks
        );
        if (ret < 0){
            break;
        }
        if (ret < option_index || (ret == option_index && mode == ClearDialogMode::STOP_CHOICES)){
            context.wait_for_all_requests();
            console.log(std::string("clear_dialog(): Detected ") + stop_description + ".");
            return;
        }
        console.log(ret == option_index
            ? "clear_dialog(): Confirming dialog option."
            : "clear_dialog(): Advancing tutorial window."
        );
        pbf_press_button(context, BUTTON_A, 100ms,
            mode == ClearDialogMode::STOP_CHOICES ? choice_release : 200ms);
        context.wait_for_all_requests();
    }

    OperationFailedExceptionWithScreenshot::fire(
        ErrorReportMode::SEND_ERROR_REPORT,
        "clear_dialog(): Did not detect the requested stop condition within " +
            std::to_string(timeout.count()) + " milliseconds.",
        console
    );
}

void press_button_until_dialog(
    ConsoleHandle& console, ProControllerContext& context, Button button,
    Milliseconds hold, Milliseconds release, Milliseconds timeout
){
    if (hold <= 0ms || release < 0ms){
        throw InternalProgramError(
            &console.logger(), PA_CURRENT_FUNCTION,
            "Button hold must be positive and release must be nonnegative."
        );
    }
    context.wait_for_all_requests();

    FlatWhiteDialogWatcher white_dialog(COLOR_WHITE, &console.overlay());
    BlueDialogWatcher blue_dialog(COLOR_BLUE, &console.overlay());
    ItemReceiveWatcher item_receive(COLOR_BLUE, &console.overlay());
    TealDialogWatcher teal_dialog(COLOR_CYAN, &console.overlay());
    LightBlueDialogWatcher light_blue_dialog(COLOR_BLUE, &console.overlay());
    TransparentBattleDialogWatcher battle_dialog(COLOR_YELLOW, &console.overlay());
    int ret = run_until<ProControllerContext>(
        console, context,
        [=](ProControllerContext& context){
            for (Milliseconds remaining = timeout; remaining > 0ms;){
                context.throw_if_cancelled();
                const Milliseconds press_hold = std::min(hold, remaining);
                const Milliseconds press_release = std::min(release, remaining - press_hold);
                pbf_press_button(context, button, press_hold, press_release);
                remaining -= press_hold + press_release;
            }
        },
        {
            white_dialog,
            blue_dialog,
            item_receive,
            teal_dialog,
            light_blue_dialog,
            battle_dialog,
        }
    );
    if (ret < 0){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "press_button_until_dialog(): No dialog detected within " +
                std::to_string(timeout.count()) + " milliseconds.",
            console
        );
    }

    context.wait_for_all_requests();
    console.log("press_button_until_dialog(): Detected dialog.");
}

void mash_button_until_right_stick(
    ConsoleHandle& console, ProControllerContext& context,
    Button mash_button, const ImageFloatBox& joystick_box,
    Milliseconds timeout
){
    context.wait_for_all_requests();

    RightStickWatcher joystick(COLOR_RED, joystick_box, &console.overlay(), 200ms);
    int ret = run_until<ProControllerContext>(
        console, context,
        [=](ProControllerContext& context){
            pbf_mash_button(context, mash_button, timeout);
        },
        {joystick}
    );
    if (ret < 0){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "mash_button_until_right_stick(): Right-stick icon not detected within " +
                std::to_string(timeout.count()) + " milliseconds.",
            console
        );
    }

    context.wait_for_all_requests();
    console.log("mash_button_until_right_stick(): Detected right-stick icon.");
}

void mash_button_until_door_entry(
    ConsoleHandle& console, ProControllerContext& context,
    Button mash_button, Milliseconds timeout
){
    context.wait_for_all_requests();

    DoorEntryWatcher door_entry(COLOR_RED, &console.overlay());
    int ret = run_until<ProControllerContext>(
        console, context,
        [=](ProControllerContext& context){
            pbf_mash_button(context, mash_button, timeout);
        },
        {door_entry}
    );
    if (ret < 0){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "mash_button_until_door_entry(): No door-entry icon detected within " +
                std::to_string(timeout.count()) + " milliseconds.",
            console
        );
    }

    context.wait_for_all_requests();
    console.log("mash_button_until_door_entry(): Detected door-entry icon.");
}

std::array<ImageMatchWatcher, 6> make_trainer_battle_watchers(Milliseconds hold_duration){
    // ImageMatchWatcher crops the same normalized box in the reference and video
    // The reference repeats a filled opponent-party icon at all six HUD slots
    static const auto reference = std::make_shared<const ImageRGB32>(
        RESOURCE_PATH() + "PokemonLZA/AutoStory/TrainerParty.png"
    );
    const auto box = [](size_t slot){
        return ImageFloatBox((846.0 + 40.8 * slot) / 1920, 60.0 / 1080, 23.0 / 1920, 24.0 / 1080);
    };
    // Watch each slot independently to disregard fainted Pokemon
    return {{
        {reference, box(0), 65, false, hold_duration, COLOR_GREEN},
        {reference, box(1), 65, false, hold_duration, COLOR_GREEN},
        {reference, box(2), 65, false, hold_duration, COLOR_GREEN},
        {reference, box(3), 65, false, hold_duration, COLOR_GREEN},
        {reference, box(4), 65, false, hold_duration, COLOR_GREEN},
        {reference, box(5), 65, false, hold_duration, COLOR_GREEN},
    }};
}

ImageFloatBox tutorial_pager_box(){
    return {0.25, 0.5, 0.5, 0.47};
}

TutorialPageWatcher::TutorialPageWatcher(Color color, VideoOverlay* overlay, Milliseconds hold_duration)
    : DetectorToFinder(
        "TutorialPageWatcher", hold_duration, color,
        ButtonType::ButtonRight, tutorial_pager_box(), overlay
    )
{}

bool TutorialPageWatcher::detect(const ImageViewRGB32& screen){
    if (ButtonDetector::detect(screen)){
        return true;
    }
    // The dark tutorial pager is the inverse of the standard button template
    // Keep the full frame dimensions so ButtonDetector's size checks and overlays stay in the original screen coordinates
    ImageRGB32 inverted = screen.copy();
    const ImagePixelBox box = floatbox_to_pixelbox(screen, tutorial_pager_box());
    for (size_t y = box.min_y; y < box.max_y; y++){
        for (size_t x = box.min_x; x < box.max_x; x++){
            inverted.pixel(x, y) ^= 0x00ffffff;
        }
    }
    return ButtonDetector::detect(inverted);
}

void run_battle_until_condition(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    BattleAIOption& battle_ai, BattleStopCondition stop_condition, Milliseconds timeout
){
    if (timeout <= 0ms){
        throw InternalProgramError(&env.console.logger(), PA_CURRENT_FUNCTION, "Battle timeout must be positive.");
    }
    const char* stop_description = nullptr;
    switch (stop_condition){
    case BattleStopCondition::STOP_OVERWORLD:
        stop_description = "overworld";
        break;
    case BattleStopCondition::STOP_BLACK_SCREEN:
        stop_description = "black screen";
        break;
    default:
        throw InternalProgramError(&env.console.logger(), PA_CURRENT_FUNCTION, "Unknown battle stop condition.");
    }
    context.wait_for_all_requests();
    const WallClock deadline = current_time() + timeout;
    TrainerBattleState battle_state(battle_ai);

    while (current_time() < deadline){
        context.throw_if_cancelled();

        OverworldPartySelectionWatcher overworld(COLOR_WHITE, &env.console.overlay());
        BlackScreenWatcher black_screen(COLOR_BLUE);
        MapWatcher map(COLOR_RED, &env.console.overlay(), 100ms);
        TutorialMapWatcher tutorial_map(COLOR_RED, &env.console.overlay());
        TutorialPageWatcher tutorial(COLOR_CYAN, &env.console.overlay());
        TutorialWindowWatcher tutorial_window(COLOR_CYAN);
        SelectionArrowWatcher dialog_option(
            COLOR_GREEN, &env.console.overlay(), SelectionArrowType::RIGHT,
            {0.5, 0.2, 0.45, 0.6}, 100ms
        );
        FlatWhiteDialogWatcher white_dialog(COLOR_WHITE, &env.console.overlay(), 100ms);
        BlueDialogWatcher blue_dialog(COLOR_BLUE, &env.console.overlay(), 100ms);
        ItemReceiveWatcher item_receive(COLOR_BLUE, &env.console.overlay(), 100ms);
        TealDialogWatcher teal_dialog(COLOR_CYAN, &env.console.overlay(), 100ms);
        LightBlueDialogWatcher light_blue_dialog(COLOR_BLUE, &env.console.overlay(), 100ms);
        TransparentBattleDialogWatcher battle_dialog(COLOR_YELLOW, &env.console.overlay(), 100ms);
        // Let dialogue detection settle before acting on the battle HUD behind it
        auto battle_watchers = make_trainer_battle_watchers(250ms);

        std::vector<PeriodicInferenceCallback> callbacks;
        if (stop_condition == BattleStopCondition::STOP_OVERWORLD){
            callbacks.emplace_back(overworld);
        }else{
            callbacks.emplace_back(black_screen);
        }
        callbacks.insert(callbacks.end(), {
            map,
            tutorial_map,
            tutorial,
            tutorial_window,
            dialog_option,
            white_dialog,
            blue_dialog,
            item_receive,
            teal_dialog,
            light_blue_dialog,
            battle_dialog,
        });
        for (auto& watcher : battle_watchers){
            callbacks.emplace_back(watcher);
        }
        int ret = wait_until(env.console, context, deadline, callbacks);
        switch (ret){
        case 0:
            env.log(std::string("run_battle_until_condition(): Detected ") + stop_description + ".");
            return;
        case 1:
        case 2:
            env.log("run_battle_until_condition(): Closing map.");
            pbf_press_button(context, BUTTON_B, 100ms, 300ms);
            break;
        case 3:
        case 4:
            env.log("run_battle_until_condition(): Advancing tutorial page.");
            pbf_press_button(context, BUTTON_A, 100ms, 200ms);
            break;
        case 5:
            env.log("run_battle_until_condition(): Confirming dialog option.");
            pbf_press_button(context, BUTTON_A, 100ms, 200ms);
            break;
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
            env.log("run_battle_until_condition(): Advancing dialog.");
            pbf_press_button(context, BUTTON_A, 100ms, 200ms);
            break;
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
            env.log("run_battle_until_condition(): Detected opponent party indicators.");
            if (!battle_state.attempt_one_attack(env, env.console, context)){
                // Send out the selected Pokemon if no attack could be launched
                pbf_press_button(context, BUTTON_UP, 500ms, 1s);
            }
            break;
        default:
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::SEND_ERROR_REPORT,
                std::string("run_battle_until_condition(): Did not detect ") + stop_description + " within " +
                    std::to_string(timeout.count()) + " milliseconds.",
                env.console
            );
        }
        context.wait_for_all_requests();
    }

    OperationFailedExceptionWithScreenshot::fire(
        ErrorReportMode::SEND_ERROR_REPORT,
        std::string("run_battle_until_condition(): Did not detect ") + stop_description + " within " +
            std::to_string(timeout.count()) + " milliseconds.",
        env.console
    );
}

void run_wild_battle_until_condition(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    BattleAIOption& battle_ai, WildBattleStopCondition stop_condition, Milliseconds timeout
){
    if (timeout <= 0ms){
        throw InternalProgramError(&env.console.logger(), PA_CURRENT_FUNCTION, "Battle timeout must be positive.");
    }
    switch (stop_condition){
    case WildBattleStopCondition::STOP_DIALOG:
        break;
    default:
        throw InternalProgramError(&env.console.logger(), PA_CURRENT_FUNCTION, "Unknown wild battle stop condition.");
    }
    context.wait_for_all_requests();
    const WallClock deadline = current_time() + timeout;
    TrainerBattleState battle_state(battle_ai);
    // Require sustained white dialogue to ignore brief background matches
    FlatWhiteDialogWatcher white_dialog(COLOR_WHITE, &env.console.overlay(), 1500ms);
    run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            while (current_time() < deadline){
                context.throw_if_cancelled();
                if (!battle_state.attempt_one_attack(env, env.console, context)){
                    pbf_wait(context, 250ms);
                }
                context.wait_for_all_requests();
            }
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::SEND_ERROR_REPORT,
                "run_wild_battle_until_condition(): No white dialogue detected within " +
                    std::to_string(timeout.count()) + " milliseconds.",
                env.console
            );
        },
        {white_dialog}
    );
    context.wait_for_all_requests();
    env.log("run_wild_battle_until_condition(): Detected white dialogue.");
}

void align_camera_left_of_booth(ConsoleHandle& console, ProControllerContext& context){
    context.wait_for_all_requests();
    const WallClock deadline = current_time() + 20s;
    BoothWatcher booth;
    // The roof overhang is wider than the wall, leaving clearance on its left
    constexpr double target_left = 0.60;
    constexpr double tolerance = 0.025;
    while (current_time() < deadline){
        context.throw_if_cancelled();
        booth.reset_state();
        if (wait_until(console, context, std::min(deadline, current_time() + 2s), {booth}) < 0){
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::SEND_ERROR_REPORT,
                "align_camera_left_of_booth(): Unable to locate the booth's cyan roof strip.",
                console
            );
        }
        const ImageFloatBox roof = booth.roof_box();
        OverlayBoxScope roof_overlay(console.overlay(), COLOR_CYAN, roof, "Booth roof");
        console.log("Booth roof left edge: " + std::to_string(roof.x));
        if (roof.x >= target_left - tolerance && roof.x <= target_left + tolerance){
            console.log("Booth aligned to the right of the forward path.");
            return;
        }
        // Looking left shifts the booth right; looking right shifts it left
        const double direction = roof.x < target_left ? -0.35 : 0.35;
        pbf_move_right_joystick(context, {direction, 0}, 80ms, 200ms);
        context.wait_for_all_requests();
    }
    OperationFailedExceptionWithScreenshot::fire(
        ErrorReportMode::SEND_ERROR_REPORT,
        "align_camera_left_of_booth(): Unable to align the booth within 20 seconds.",
        console
    );
}

void center_event_marker(ConsoleHandle& console, ProControllerContext& context){
    context.wait_for_all_requests();
    const WallClock deadline = current_time() + 30s;
    EventMarkerWatcher marker;
    CenteredEventMarkerWatcher centered;
    constexpr double tolerance = 0.02;
    bool seen_marker = false;
    double last_center = 0.5;
    double last_direction = 1;
    size_t recovery_turns = 0;
    while (current_time() < deadline){
        context.throw_if_cancelled();
        marker.reset_state();
        if (wait_until(console, context, std::min(deadline, current_time() + 750ms), {marker}) < 0){
            if (seen_marker){
                if (recovery_turns >= 4){
                    OperationFailedExceptionWithScreenshot::fire(
                        ErrorReportMode::SEND_ERROR_REPORT,
                        "center_event_marker(): Unable to recover the event marker after four camera turns.",
                        console
                    );
                }
                // The flag transition can move the badge off the screen edge
                // Keep turning toward the last known target to bring it back
                recovery_turns++;
                const bool far_from_center = std::abs(last_center - 0.5) > 0.25;
                const double strength = far_from_center ? 0.65 : 0.35;
                const Milliseconds hold = far_from_center ? 160ms : 60ms;
                console.log("Recovering event marker in the last known direction (" +
                    std::to_string(recovery_turns) + " of 4).");
                pbf_move_right_joystick(context, {last_direction * strength, 0}, hold, 250ms);
            }else{
                // Search around the player when the gate starts outside the view
                pbf_move_right_joystick(context, {0.65, 0}, 250ms, 250ms);
            }
        }else{
            seen_marker = true;
            recovery_turns = 0;
            const ImageFloatBox box = marker.marker_box();
            OverlayBoxScope marker_overlay(console.overlay(), COLOR_YELLOW, box, "Event marker");
            const double center = box.x + box.width / 2;
            last_center = center;
            last_direction = center < 0.5 ? -1 : 1;
            console.log("Event marker center: " + std::to_string(center));
            if (center >= 0.5 - tolerance && center <= 0.5 + tolerance){
                centered.reset_state();
                if (wait_until(console, context, std::min(deadline, current_time() + 1s), {centered}) >= 0){
                    console.log("Event marker held at center. Ready to walk forward.");
                    return;
                }
                // Re-read its position if the graphic moves during confirmation
                continue;
            }
            // Large turns at the edge, then progressively finer corrections
            const double error = std::abs(center - 0.5);
            double strength;
            Milliseconds hold;
            if (error > 0.25){
                strength = 0.65;
                hold = 220ms;
            }else if (error > 0.10){
                strength = 0.5;
                hold = 120ms;
            }else{
                strength = 0.35;
                hold = 40ms;
            }
            pbf_move_right_joystick(context, {last_direction * strength, 0}, hold, 250ms);
        }
        context.wait_for_all_requests();
    }
    OperationFailedExceptionWithScreenshot::fire(
        ErrorReportMode::SEND_ERROR_REPORT,
        "center_event_marker(): Unable to find and center the event marker within 30 seconds.",
        console
    );
}

bool party_has_at_least(ConsoleHandle& console, ProControllerContext& context, uint8_t count){
    if (count == 0 || count > 6){
        throw InternalProgramError(&console.logger(), PA_CURRENT_FUNCTION, "Party size must be between 1 and 6.");
    }
    context.wait_for_all_requests();
    OverworldPartySelectionWatcher party(COLOR_WHITE, &console.overlay(), 250ms);
    if (wait_until(console, context, 5s, {party}) < 0){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "party_has_at_least(): Unable to read the selected party Pokemon.",
            console
        );
    }
    const uint8_t original = party.selected_party_idx();
    bool found = original >= count - 1;
    // Cycle back to the original selection so later inputs use the same Pokemon
    for (size_t step = 0; step < 6; step++){
        pbf_press_dpad(context, DPAD_RIGHT, 100ms, 300ms);
        context.wait_for_all_requests();
        party.reset_state();
        if (wait_until(console, context, 5s, {party}) < 0){
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::SEND_ERROR_REPORT,
                "party_has_at_least(): Unable to read the party after switching Pokemon.",
                console
            );
        }
        const uint8_t selected = party.selected_party_idx();
        found |= selected >= count - 1;
        if (selected == original){
            console.log("party_has_at_least(): " + std::string(found ? "Found" : "Did not find") +
                " at least " + std::to_string(count) + " party Pokemon.");
            return found;
        }
    }
    OperationFailedExceptionWithScreenshot::fire(
        ErrorReportMode::SEND_ERROR_REPORT,
        "party_has_at_least(): Party selection did not return to its starting slot.",
        console
    );
}

void run_catch(ProControllerContext& context, Milliseconds post_throw_wait){
    if (post_throw_wait < 0ms){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Post-throw wait must be nonnegative.");
    }
    ssf_press_button(context, BUTTON_ZL | BUTTON_ZR, 500ms, 500ms, 0ms);
    pbf_wait(context, post_throw_wait);
    context.wait_for_all_requests();
}

void overworld_navigation(
    ConsoleHandle& console, ProControllerContext& context,
    NavigationStopCondition stop_condition, NavigationMovementMode movement_mode,
    double direction_x, double direction_y, Milliseconds timeout, bool sprint,
    ButtonType button_type, const ImageFloatBox& button_box
){
    if (timeout <= 0ms){
        throw InternalProgramError(&console.logger(), PA_CURRENT_FUNCTION, "Navigation timeout must be positive.");
    }
    switch (movement_mode){
    case NavigationMovementMode::DIRECTIONAL_ONLY:
    case NavigationMovementMode::DIRECTIONAL_SPAM_A:
        break;
    default:
        throw InternalProgramError(&console.logger(), PA_CURRENT_FUNCTION, "Unknown navigation movement mode.");
    }
    context.wait_for_all_requests();

    FlatWhiteDialogWatcher white_dialog(COLOR_WHITE, &console.overlay());
    BlueDialogWatcher blue_dialog(COLOR_BLUE, &console.overlay());
    ItemReceiveWatcher item_receive(COLOR_BLUE, &console.overlay());
    TealDialogWatcher teal_dialog(COLOR_CYAN, &console.overlay());
    LightBlueDialogWatcher light_blue_dialog(COLOR_BLUE, &console.overlay());
    TransparentBattleDialogWatcher battle_dialog(COLOR_YELLOW, &console.overlay());
    ButtonWatcher button(COLOR_RED, button_type, button_box, &console.overlay(), 200ms);
    DoorEntryWatcher door_entry(COLOR_RED, &console.overlay());
    auto battle_watchers = make_trainer_battle_watchers(100ms);
    BlackScreenOverWatcher black_screen_over(COLOR_BLUE);
    std::vector<PeriodicInferenceCallback> callbacks;
    const char* stop_description = nullptr;
    switch (stop_condition){
    case NavigationStopCondition::STOP_DIALOG:
        callbacks = {
            white_dialog,
            blue_dialog,
            item_receive,
            teal_dialog,
            light_blue_dialog,
            battle_dialog,
        };
        stop_description = "dialog";
        break;
    case NavigationStopCondition::STOP_BUTTON:
        callbacks.emplace_back(button);
        stop_description = "requested button";
        break;
    case NavigationStopCondition::STOP_DOOR_ENTRY:
        callbacks.emplace_back(door_entry);
        stop_description = "door-entry icon";
        break;
    case NavigationStopCondition::STOP_TIME:
        stop_description = "elapsed time";
        break;
    case NavigationStopCondition::STOP_BATTLE:
        for (auto& watcher : battle_watchers){
            callbacks.emplace_back(watcher);
        }
        stop_description = "opponent party indicators";
        break;
    case NavigationStopCondition::STOP_BLACK_SCREEN:
        callbacks.emplace_back(black_screen_over);
        stop_description = "end of black-screen transition";
        break;
    default:
        throw InternalProgramError(&console.logger(), PA_CURRENT_FUNCTION, "Unknown navigation stop condition.");
    }

    int ret = run_until<ProControllerContext>(
        console, context,
        [=](ProControllerContext& context){
            if (sprint){
                ssf_press_button(context, BUTTON_B, 0ms, timeout, 0ms);
            }
            ssf_press_left_joystick(context, {direction_x, direction_y}, 0ms, timeout, 0ms);
            if (movement_mode == NavigationMovementMode::DIRECTIONAL_ONLY){
                pbf_wait(context, timeout);
            }else{
                for (Milliseconds remaining = timeout; remaining > 0ms;){
                    context.throw_if_cancelled();
                    const Milliseconds hold = std::min(160ms, remaining);
                    const Milliseconds release = std::min(40ms, remaining - hold);
                    pbf_press_button(context, BUTTON_A, hold, release);
                    remaining -= hold + release;
                }
            }
        },
        callbacks
    );
    if (ret < 0 && stop_condition != NavigationStopCondition::STOP_TIME){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            std::string("overworld_navigation(): Did not detect ") + stop_description + " within " +
                std::to_string(timeout.count()) + " milliseconds.",
            console
        );
    }

    context.wait_for_all_requests();
    if (stop_condition == NavigationStopCondition::STOP_TIME){
        console.log("overworld_navigation(): Finished timed movement.");
    }else{
        console.log(std::string("overworld_navigation(): Detected ") + stop_description + ".");
    }
}

}
}
}
