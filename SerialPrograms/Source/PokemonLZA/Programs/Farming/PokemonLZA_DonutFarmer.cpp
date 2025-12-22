/*  Donut Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA/Programs/PokemonLZA_GameEntry.h"
#include "PokemonLZA_DonutFarmer.h"
#include "Common/Cpp/Options/ButtonOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;


DonutFarmer_Descriptor::DonutFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
          "PokemonLZA:DonutFarmer",
          STRING_POKEMON + " LZA", "Donut Farmer",
          "Programs/PokemonLZA/MegaShardFarmer.html",
          "Farm donut from Pokemon Center to Hotel Z.",
          ProgramControllerClass::StandardController_NoRestrictions,
          FeedbackType::REQUIRED,
          AllowCommandsWhenRunning::DISABLE_COMMANDS,
          {}
          )
{}
class DonutFarmer_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : rounds(m_stats["Rounds"])
        , donuts(m_stats["Saved Donut"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Rounds");
        m_display_order.emplace_back("Saved Donut");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& rounds;
    std::atomic<uint64_t>& donuts;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> DonutFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

DonutFarmer::DonutFarmer()
    : DONUT_OPTION(
          "<b>Choose Recepie</b>",
          {
           { DonutOption::Kasib, "Kasib", "Kasib" },
           { DonutOption::Tanga, "Tanga", "Tanga" },
           { DonutOption::Battle_Zone_1, "Battle Zone 1", "Battle Zone 1" },
           { DonutOption::Battle_Zone_2, "Battle Zone 2", "Battle Zone 2" },
           { DonutOption::Item_Power_1, "Item Power 1", "Item Power 1" },
           { DonutOption::Item_Power_2, "Item Power 2", "Item Power 2" },
           { DonutOption::Sparkling_Power_1, "Sparkling Power 1", "Sparkling Power 1" },
           { DonutOption::Sparkling_Power_2, "Sparkling Power 2", "Sparkling Power 2" },
           { DonutOption::Sparkling_Power_3, "Sparkling Power 3", "Sparkling Power 3" },
           { DonutOption::Cherry, "Cherry", "Cherry" },
           },
          LockMode::LOCK_WHILE_RUNNING,
          DonutOption::Kasib
          )
    , SAVE_AND_RESUME("<b>Save Donut</b>", "Save current donut and fast travel to the PokeCenter and resume farmer")
    , RESET_WITHOUT_SAVING("<b>Reset without saving</b>", "Discard current donut and resume farmer")
{
    {
        PA_ADD_OPTION(DONUT_OPTION);
        PA_ADD_OPTION(SAVE_AND_RESUME);
        PA_ADD_OPTION(RESET_WITHOUT_SAVING);
    }
}

//Move joystick in a direction steps amount of time

void DonutFarmer::move_joystick_steps(ProControllerContext& context, uint8_t x, uint8_t y, int steps) {
    for (int i = 0; i < steps; i++) {
        pbf_move_left_joystick(context, x, y, 100ms, 250ms);
    }
}

//Press Button steps amount of time

void DonutFarmer::press_button_steps(ProControllerContext& context, Button button, int steps) {
    for (int i = 0; i < steps; i++) {
        pbf_press_button(context, button, 100ms, 250ms);
    }
}

void DonutFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    // Button listener and flag declaration

    MyButtonListener save_listener(save_requested);
    MyButtonListener reset_listener(reset_requested);
    SAVE_AND_RESUME.add_listener(save_listener);
    RESET_WITHOUT_SAVING.add_listener(reset_listener);
    save_requested = false;
    reset_requested = false;

    //reset and load backup save

    reset_game_from_home(env, env.console, context, true, 8000ms);

    while(true){

        DonutFarmer_Descriptor::Stats& stats = env.current_stats<DonutFarmer_Descriptor::Stats>();
        stats.rounds++;
        env.update_stats();

        //open map

        open_map(env.console, context);

        //fly to Hotel Z

        pbf_wait(context, 1500ms);
        pbf_move_left_joystick(context, 0, 128, 276ms, 0ms);
        pbf_wait(context, 500ms);
        pbf_mash_button(context, BUTTON_A, 800ms);
        pbf_wait(context, 5000ms);

        //run to the door

        wait_until_overworld(env.console, context);
        run_a_straight_path_in_overworld(env.console, context, 128, 0, 900ms);

        //open door

        wait_until_overworld(env.console, context);
        pbf_press_button(context, BUTTON_A, 150ms, 0ms);
        pbf_wait(context, 1000ms);

        //run towards Ansha and then make a left turn to face her

        wait_until_overworld(env.console, context);
        run_a_straight_path_in_overworld(env.console, context, 128, 0, 900ms);
        pbf_wait(context, 1000ms);
        wait_until_overworld(env.console, context);
        run_a_straight_path_in_overworld(env.console, context, 128, 0, 1600ms);
        pbf_move_left_joystick(context, 0, 0, 70ms, 0ms);
        pbf_move_left_joystick(context, 0, 128, 400ms, 0ms);

        //talk to Ansha and open berry menu

        pbf_wait(context, 500ms);
        pbf_press_button(context, BUTTON_A, 250ms, 0ms);
        pbf_wait(context, 500ms);
        pbf_press_button(context, BUTTON_A, 250ms, 0ms);
        pbf_wait(context, 500ms);
        pbf_press_button(context, BUTTON_A, 250ms, 0ms);
        pbf_wait(context, 500ms);

        //move 1 up to go to end of berry list

        pbf_move_left_joystick(context, 128, 0, 100ms, 0ms);
        pbf_wait(context, 250ms);

        //switch case with varius recepie

        switch(DONUT_OPTION){
        case DonutOption::Kasib:
            move_joystick_steps(context, 128, 0, 5);
            press_button_steps(context, BUTTON_A, 8);
            break;
        case DonutOption::Tanga:
            move_joystick_steps(context, 128, 0, 7);
            press_button_steps(context, BUTTON_A, 8);
            break;
        case DonutOption::Battle_Zone_1:
            move_joystick_steps(context, 128, 0, 6);
            press_button_steps(context, BUTTON_A, 4);
            move_joystick_steps(context, 128, 0, 3);
            press_button_steps(context, BUTTON_A, 4);
            break;
        case DonutOption::Battle_Zone_2:
            move_joystick_steps(context, 128, 0, 4);
            press_button_steps(context, BUTTON_A, 2);
            move_joystick_steps(context, 128, 255, 2);
            press_button_steps(context, BUTTON_A, 2);
            move_joystick_steps(context, 128, 255, 1);
            press_button_steps(context, BUTTON_A, 2);
            move_joystick_steps(context, 128, 255, 1);
            press_button_steps(context, BUTTON_A, 2);
            break;
        case DonutOption::Item_Power_1:
            move_joystick_steps(context, 128, 0, 8);
            press_button_steps(context, BUTTON_A, 1);
            move_joystick_steps(context, 128, 255, 2);
            press_button_steps(context, BUTTON_A, 1);
            move_joystick_steps(context, 128, 255, 3);
            press_button_steps(context, BUTTON_A, 4);
            move_joystick_steps(context, 128, 255, 3);
            press_button_steps(context, BUTTON_A, 2);
            break;
        case DonutOption::Item_Power_2:
            move_joystick_steps(context, 128, 0, 8);
            press_button_steps(context, BUTTON_A, 1);
            move_joystick_steps(context, 128, 255, 2);
            press_button_steps(context, BUTTON_A, 1);
            move_joystick_steps(context, 128, 255, 4);
            press_button_steps(context, BUTTON_A, 4);
            move_joystick_steps(context, 128, 255, 2);
            press_button_steps(context, BUTTON_A, 2);
            break;
        case DonutOption::Sparkling_Power_1:
            move_joystick_steps(context, 128, 0, 8);
            press_button_steps(context, BUTTON_A, 1);
            move_joystick_steps(context, 128, 255, 2);
            press_button_steps(context, BUTTON_A, 1);
            move_joystick_steps(context, 128, 255, 2);
            press_button_steps(context, BUTTON_A, 5);
            move_joystick_steps(context, 128, 255, 2);
            press_button_steps(context, BUTTON_A, 1);
            break;
        case DonutOption::Sparkling_Power_2:
            move_joystick_steps(context, 128, 0, 8);
            press_button_steps(context, BUTTON_A, 1);
            move_joystick_steps(context, 128, 255, 2);
            press_button_steps(context, BUTTON_A, 1);
            move_joystick_steps(context, 128, 255, 2);
            press_button_steps(context, BUTTON_A, 5);
            move_joystick_steps(context, 128, 255, 3);
            press_button_steps(context, BUTTON_A, 1);
            break;
        case DonutOption::Sparkling_Power_3:
            move_joystick_steps(context, 128, 0, 8);
            press_button_steps(context, BUTTON_A, 1);
            move_joystick_steps(context, 128, 255, 2);
            press_button_steps(context, BUTTON_A, 1);
            move_joystick_steps(context, 128, 255, 2);
            press_button_steps(context, BUTTON_A, 5);
            move_joystick_steps(context, 128, 255, 4);
            press_button_steps(context, BUTTON_A, 1);
            break;
        case DonutOption::Cherry:
            move_joystick_steps(context, 128, 255, 1);
            press_button_steps(context, BUTTON_A, 3);
            break;
        }

        //Iniziate donut crafting

        pbf_wait(context, 1500ms);
        pbf_press_button(context, BUTTON_PLUS, 150ms, 0ms);
        pbf_wait(context, 2000ms);
        pbf_mash_button(context, BUTTON_A, 5000ms);

        while (true) {

            // Wait for player input
            while (!save_requested && !reset_requested) {
                context.wait_for(std::chrono::milliseconds(100));
            }

            if (save_requested) {
                save_requested = false;

                //exit Ansha menu

                pbf_wait(context, 500ms);
                pbf_press_button(context, BUTTON_A, 250ms, 0ms);
                pbf_wait(context, 500ms);
                pbf_mash_button(context, BUTTON_B, 5000ms);

                open_map(env.console, context);

                //fly to PokeCenter to save inventory

                pbf_wait(context, 1500ms);
                pbf_move_left_joystick(context, 255, 128, 276ms, 0ms);
                pbf_wait(context, 500ms);
                pbf_mash_button(context, BUTTON_A, 800ms);
                pbf_wait(context, 5000ms);

                stats.donuts++;

                //reset and load save to keep the donut

                reset_game_from_home(env, env.console, context, false, 8000ms);
                break;
            } else {
                reset_requested = false;

                //reset and load backup save to discard donut

                reset_game_from_home(env, env.console, context, true, 8000ms);
                break;
            }

        }
    }
}
}
}
}
