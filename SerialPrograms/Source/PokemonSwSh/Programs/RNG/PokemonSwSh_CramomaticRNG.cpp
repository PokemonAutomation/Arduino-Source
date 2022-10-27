/*  Cram-o-matic RNG Manipulation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Credit goes to Anubis for discovering how the Cram-o-matic works
 *  and for the original code to calculate how many advances are needed
 *  to get the wanted balls.
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/DebugDumper.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "PokemonSwSh/Programs/RNG/PokemonSwSh_BasicRNG.h"
#include "PokemonSwSh/Programs/RNG/PokemonSwSh_CramomaticRNG.h"

#include <algorithm>

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonSwSh {
using namespace Pokemon;

CramomaticRNG_Descriptor::CramomaticRNG_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:CramomaticRNG",
        STRING_POKEMON + " SwSh", "Cram-o-matic RNG",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/CramomaticRNG.md",
        "Perform RNG manipulation to get rare balls from the Cram-o-matic.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

CramomaticRNG::CramomaticRNG()
    : NUM_APRICORN_ONE(
        "<b>Primary Apricorns:</b><br>Number of Apricorns in the selected slot.",
        0
    )
    , NUM_APRICORN_TWO(
        "<b>Secondary Apricorns:</b><br>Number of Apricorns in the slot below the selected one.",
        0
    )
    , NUM_NPCS(
        "<b>NPCs:</b><br>Number of NPCs in the dojo, including " + STRING_POKEMON + ".",
        21
    )
    , BALL_TABLE(
        "<b>Wanted Balls:</b><br>Exact kind depends on the currently selected apricorn."
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , MAX_PRIORITY_ADVANCES(
        "<b>Priority Advances:</b><br>How many advances to check when checking for higher priority selections.",
        300
    )
    , MAX_UNKNOWN_ADVANCES(
        "<b>Max Unknown advances:</b><br>How many advances to check when updating the rng state.",
        300
    )
    , SAVE_SCREENSHOTS(
        "<b>Save Debug Screenshots:</b>",
        LockWhileRunning::LOCK_WHILE_RUNNING,
        false
    )
    , LOG_VALUES(
        "<b>Log Animation Values:</br>",
        LockWhileRunning::LOCK_WHILE_RUNNING,
        false
    )

{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(NUM_APRICORN_ONE);
    PA_ADD_OPTION(NUM_APRICORN_TWO);
    PA_ADD_OPTION(NUM_NPCS);
    PA_ADD_OPTION(BALL_TABLE);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(MAX_PRIORITY_ADVANCES);
    PA_ADD_OPTION(MAX_UNKNOWN_ADVANCES);
    PA_ADD_OPTION(SAVE_SCREENSHOTS);
    PA_ADD_OPTION(LOG_VALUES);
}

void CramomaticRNG::navigate_to_party(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
    pbf_press_button(context, BUTTON_X, 10, 125);
    pbf_press_button(context, BUTTON_A, 10, 10);
    pbf_wait(context, 2 * TICKS_PER_SECOND);
}

CramomaticTarget CramomaticRNG::calculate_target(SingleSwitchProgramEnvironment& env, Xoroshiro128PlusState state, std::vector<CramomaticSelection> selected_balls) {
    Xoroshiro128Plus rng(state);
    size_t advances = 0;
    uint16_t priority_advances = 0;
    std::vector<CramomaticTarget> possible_targets;

    std::sort(selected_balls.begin(), selected_balls.end(), [](CramomaticSelection sel1, CramomaticSelection sel2) { return sel1.priority > sel2.priority; });

    // priority_advances only starts counting up after the first good result is found
    while (priority_advances <= MAX_PRIORITY_ADVANCES) {
        // calculate the result for the current temp_rng state
        Xoroshiro128Plus temp_rng(rng.get_state());

        for (size_t i = 0; i < NUM_NPCS; i++) {
            temp_rng.nextInt(91);
        }
        temp_rng.next();
        temp_rng.nextInt(60);

        /*uint64_t item_roll =*/ temp_rng.nextInt(4);
        uint64_t ball_roll = temp_rng.nextInt(100);
        bool is_safari_sport = temp_rng.nextInt(1000) == 0;
        bool is_bonus = false;

        if (is_safari_sport || ball_roll == 99) {
            is_bonus = temp_rng.nextInt(1000) == 0;
        }
        else {
            is_bonus = temp_rng.nextInt(100) == 0;
        }

        CramomaticBallType type;
        if (is_safari_sport) {
            type = CramomaticBallType::Safari;
        }
        else if (ball_roll < 25) {
            type = CramomaticBallType::Poke;
        }
        else if (ball_roll < 50) {
            type = CramomaticBallType::Great;
        }
        else if (ball_roll < 75) {
            type = CramomaticBallType::Shop1;
        }
        else if (ball_roll < 99) {
            type = CramomaticBallType::Shop2;
        }
        else {
            type = CramomaticBallType::Apricorn;
        }
        

        // check whether the result is a good result
        for (size_t i = 0; i < selected_balls.size(); i++) {
            CramomaticSelection selection = selected_balls[i];
            if (!selection.is_bonus || is_bonus) {
                if (is_safari_sport) {
                    if (selection.ball_type == CramomaticBallType::Safari || selection.ball_type == CramomaticBallType::Sport) {
                        type = selection.ball_type;
                    }
                }
                
                if (selection.ball_type == type) {
                    CramomaticTarget target;
                    target.ball_type = type;
                    target.is_bonus = is_bonus;
                    target.needed_advances = advances;
                    possible_targets.emplace_back(target);

                    priority_advances = 0;

                    uint16_t priority = selection.priority;
                    selected_balls.erase(
                        std::remove_if(selected_balls.begin(), selected_balls.end()
                            , [priority](CramomaticSelection sel) { return sel.priority <= priority; })
                        , selected_balls.end());
                    break;
                }

            }
        }
        if (possible_targets.size() > 0) {
            if (selected_balls.empty()) {
                //priority_advances = MAX_PRIORITY_ADVANCES + 1;
                break;
            }
            priority_advances++;
        }

        rng.next();
        advances++;
    }


    while (possible_targets.size() > 1) {
        auto last_target = possible_targets.end() - 1;
        auto second_to_last_target = possible_targets.end() - 2;

        if ((*last_target).needed_advances - (*second_to_last_target).needed_advances > MAX_PRIORITY_ADVANCES) {
            possible_targets.erase(last_target);
        }
        else {
            possible_targets.erase(second_to_last_target);
        }
    }

    return possible_targets[0];
}

void CramomaticRNG::leave_to_overworld_and_interact(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
    pbf_press_button(context, BUTTON_B, 2 * TICKS_PER_SECOND, 5);
    pbf_press_button(context, BUTTON_B, 10, 70);

    pbf_mash_button(context, BUTTON_A, 320);
    pbf_wait(context, 125);
}

void CramomaticRNG::choose_apricorn(SingleSwitchProgramEnvironment& env, BotBaseContext& context, bool sport) {
    // check whether the bag is open
    context.wait_for_all_requests();
    VideoOverlaySet boxes(env.console);
    SelectionArrowFinder bag_arrow_detector(env.console, ImageFloatBox(0.465, 0.195, 0.054, 0.57));
    bag_arrow_detector.make_overlays(boxes);

    int ret = wait_until(env.console, context, Milliseconds(5000), { bag_arrow_detector });
    if (ret < 0) {
        throw OperationFailedException(env.console, "Could not detect bag.");
    }

    // select the apricorn(s)
    pbf_wait(context, 1 * TICKS_PER_SECOND);
    pbf_press_button(context, BUTTON_A, 10, 30);
    if (sport) {
        pbf_press_dpad(context, DPAD_DOWN, 20, 10);
    }
    pbf_press_button(context, BUTTON_A, 10, 30);
    pbf_press_button(context, BUTTON_A, 10, 30);
    if (sport) {
        pbf_press_dpad(context, DPAD_UP, 20, 10);
    }
    pbf_press_button(context, BUTTON_A, 10, 30);

    pbf_mash_button(context, BUTTON_A, 5 * TICKS_PER_SECOND);
}

bool CramomaticRNG::receive_ball(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
    // receive ball and refuse to use the cram-o-matic again
    VideoOverlaySet boxes(env.console);
    SelectionArrowFinder arrow_detector(env.console, ImageFloatBox(0.350, 0.450, 0.500, 0.400));
    arrow_detector.make_overlays(boxes);
    size_t presses = 0;
    bool arrow_detected = false;
    while (presses < 30 && !arrow_detected) {
        presses++;
        pbf_press_button(context, BUTTON_B, 10, 165);
        context.wait_for_all_requests();

        std::shared_ptr<const ImageRGB32> screen = env.console.video().snapshot();
        if (SAVE_SCREENSHOTS) {
            dump_debug_image(env.logger(), "cramomatic-rng", "receive", *screen);
        }
        if (arrow_detector.detect(*screen)) {
            arrow_detected = true;
        }
    }
    pbf_press_button(context, BUTTON_B, 10, TICKS_PER_SECOND);
    return arrow_detected;
}


void CramomaticRNG::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
    std::vector<CramomaticSelection> selections = BALL_TABLE.selected_balls();
    if (selections.empty()) {
        throw UserSetupError(env.console, "At least one type of ball needs to be selected!");
    }


    if (START_LOCATION.start_in_grip_menu()) {
        grip_menu_connect_go_home(context);
        PokemonSwSh::resume_game_back_out(context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    }
    else {
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    Xoroshiro128Plus rng(0, 0);
    bool is_state_valid = false;
    uint32_t num_apricorn_one = NUM_APRICORN_ONE;
    uint32_t num_apricorn_two = NUM_APRICORN_TWO;

    // if there is no Sport Ball in the selected balls we ignore num_apricorn_two
    bool sport_wanted = false;
    for (CramomaticSelection selection : selections) {
        if (selection.ball_type == CramomaticBallType::Sport) {
            sport_wanted = true;
            break;
        }
    }

    size_t iteration = 0;
    while (num_apricorn_one > 4 && (!sport_wanted || num_apricorn_two > 2)) {
        env.console.log("Cram-o-matic RNG iteration: " + std::to_string(iteration));
        navigate_to_party(env, context);
        context.wait_for_all_requests();

        if (!is_state_valid) {
            rng = Xoroshiro128Plus(find_rng_state(env.console, context, SAVE_SCREENSHOTS, LOG_VALUES));
            is_state_valid = true;
        }
        else {
            rng = Xoroshiro128Plus(refind_rng_state(env.console, context, rng.get_state(), 0, MAX_UNKNOWN_ADVANCES, SAVE_SCREENSHOTS, LOG_VALUES));
        }
        Xoroshiro128PlusState rng_state = rng.get_state();
        if (rng_state.s0 == 0 && rng_state.s1 == 0) {
            throw OperationFailedException(env.console, "Invalid RNG state detected.");
        }

        CramomaticTarget target = calculate_target(env, rng.get_state(), BALL_TABLE.selected_balls());
        bool sport = target.ball_type == CramomaticBallType::Sport;
        env.console.log("Needed advances: " + std::to_string(target.needed_advances));
        num_apricorn_one -= sport ? 2 : 4;
        num_apricorn_two -= sport ? 2 : 0;

        do_rng_advances(env.console, context, rng, target.needed_advances);
        leave_to_overworld_and_interact(env, context);
        choose_apricorn(env, context, sport);

        bool did_refuse = receive_ball(env, context);
        if (!did_refuse) {
            is_state_valid = false;
        }

        iteration++;
    }
}


}
}
}
