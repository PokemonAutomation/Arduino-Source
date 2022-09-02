/*  Cram-o-matic RNG Manipulation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *  Credit goes to Anubis for discovering how the Cram-o-matic works 
 *  and for the original code to calculate how many advances are needed 
 *  to get the wanted balls.
 * 
 */

#include "Common/Cpp/PrettyPrint.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "PokemonSwSh/Programs/RNG/PokemonSwSh_BasicRNG.h"
#include "PokemonSwSh/Programs/RNG/PokemonSwSh_CramomaticRNG.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"

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
                : NUM_ITERATIONS(
                    "<b>Iterations:</b><br>How often should the Cram-o-matic be used. Four apricorns are used per iteration.</b>",
                    240
                )
                , NUM_NPCS(
                    "<b>NPCs:</b><br>Number of NPCs in the dojo, including " + STRING_POKEMON + ".",
                    22
                )
                , BALL_TYPE(
                    "<b>Wanted Ball:</b><br>Exact kind depends on the currently selected apricorn.",
                    {
                        {BallType::Poke, "poke", "Poke Ball"},
                        {BallType::Great, "great", "Great Ball"},
                        {BallType::Shop1, "shop1", "Ultra Ball, Net Ball, Dusk Ball, Premier Ball"},
                        {BallType::Shop2, "shop2", "Repeat Ball, Dive Ball, Quick Ball, Nest Ball, Heal Ball, Timer Ball, Luxury Ball"},
                        {BallType::Apricorn, "apricorn", "Level Ball, Lure Ball, Moon Ball, Friend Ball, Love Ball, Fast Ball, Heavy Ball"},
                        {BallType::Safari, "safari", "Safari Ball"},
                        {BallType::Sport, "sport", "Sport Ball (uses two different Apricorn colors)"},
                    },
                    BallType::Apricorn
                    )
                , m_advanced_options(
                    "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
                )
                , MAX_UNKNOWN_ADVANCES(
                    "<b>Max unknown advances:</b><br>How many advances to check when updating the rng state.",
                    300
                )
                , ONLY_BONUS(
                    "<b>Only bonus:</b>",
                    false
                )
                , SAVE_SCREENSHOTS(
                    "<b>Save debug screenshots:</b>",
                    false
                )
                , LOG_VALUES(
                    "<b>Log animation values:</br>",
                    false
                )

            {
                PA_ADD_OPTION(START_LOCATION);
                PA_ADD_OPTION(NUM_ITERATIONS);
                PA_ADD_OPTION(NUM_NPCS);
                PA_ADD_OPTION(BALL_TYPE);

                PA_ADD_STATIC(m_advanced_options);
                PA_ADD_OPTION(MAX_UNKNOWN_ADVANCES);
                PA_ADD_OPTION(ONLY_BONUS);
                PA_ADD_OPTION(SAVE_SCREENSHOTS);
                PA_ADD_OPTION(LOG_VALUES);
            }


            void CramomaticRNG::navigate_to_party(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
                pbf_press_button(context, BUTTON_X, 10, 125);
                pbf_press_button(context, BUTTON_A, 10, 10);
                pbf_wait(context, 2 * TICKS_PER_SECOND);
            }

            size_t CramomaticRNG::needed_advances(SingleSwitchProgramEnvironment& env, Xoroshiro128PlusState state, BallType wanted_type) {
                bool is_wanted_type = false;
                Xoroshiro128Plus rng(state);
                size_t advances = 0;

                while (!is_wanted_type) {
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

                    if (ONLY_BONUS && !is_bonus) {
                        continue;
                    }

                    switch (wanted_type)
                    {
                    case BallType::Poke:
                        is_wanted_type = ball_roll < 25 && !is_safari_sport;
                        break;
                    case BallType::Great:
                        is_wanted_type = ball_roll >= 25 && ball_roll < 50 && !is_safari_sport;
                        break;
                    case BallType::Shop1:
                        is_wanted_type = ball_roll >= 50 && ball_roll < 75 && !is_safari_sport;
                        break;
                    case BallType::Shop2:
                        is_wanted_type = ball_roll >= 75 && ball_roll < 99 && !is_safari_sport;
                        break;
                    case BallType::Apricorn:
                        is_wanted_type = ball_roll == 99 && !is_safari_sport;
                        break;
                    case BallType::Safari:
                    case BallType::Sport:
                        is_wanted_type = is_safari_sport;
                        break;
                    }

                    rng.next();
                    advances++;
                }
                return advances - 1;
            }

            void CramomaticRNG::leave_to_overworld_and_interact(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
                pbf_press_button(context, BUTTON_B, 2 * TICKS_PER_SECOND, 5);
                pbf_press_button(context, BUTTON_B, 10, 70);

                pbf_mash_button(context, BUTTON_A, 300);
                pbf_wait(context, 125);
            }

            void CramomaticRNG::choose_apricorn(SingleSwitchProgramEnvironment& env, BotBaseContext& context, bool sport) {
                pbf_press_button(context, BUTTON_A, 10, 25);
                if (sport) {
                    pbf_press_dpad(context, DPAD_DOWN, 20, 10);
                }
                pbf_press_button(context, BUTTON_A, 10, 25);
                pbf_press_button(context, BUTTON_A, 5, 30);
                if (sport) {
                    pbf_press_dpad(context, DPAD_UP, 20, 10);
                }
                pbf_press_button(context, BUTTON_A, 10, 25);

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
                        screen->save(now_to_filestring() + ".png");
                    }
                    if (arrow_detector.detect(*screen)) {
                        arrow_detected = true;
                    }
                }
                pbf_press_button(context, BUTTON_B, 10, TICKS_PER_SECOND);
                return arrow_detected;
            }


            void CramomaticRNG::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
                if (START_LOCATION.start_in_grip_menu()) {
                    grip_menu_connect_go_home(context);
                    PokemonSwSh::resume_game_back_out(context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
                }
                else {
                    pbf_press_button(context, BUTTON_B, 5, 5);
                }

                Xoroshiro128Plus rng(0, 0);
                bool is_state_valid = false;
                bool sport = BALL_TYPE == BallType::Sport;

                size_t iteration = 0;
                while (iteration < NUM_ITERATIONS) {
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

                    size_t advances = needed_advances(env, rng.get_state(), BALL_TYPE);
                    env.console.log("Needed advances: " + std::to_string(advances));

                    do_rng_advances(env.console, context, rng, advances);
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
