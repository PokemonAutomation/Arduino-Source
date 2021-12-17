/*  Activate Menu Glitch (Poketch)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MapDetector.h"
#include "PokemonBDSP_ActivateMenuGlitch-Poketch.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


ActivateMenuGlitchPoketch_Descriptor::ActivateMenuGlitchPoketch_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonBDSP:ActivateMenuGlitchPoketch",
        STRING_POKEMON + " BDSP", QString("Activate Menu Glitch (Pok") + QChar(0xe9) + "tch)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/ActivateMenuGlitch-Poketch.md",
        QString("Activate the menu glitch using the Pok") + QChar(0xe9) + "tch."
        "<font color=\"red\">This method works on v1.1.2 and earlier. It may be patched out in the future.</font>",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

ActivateMenuGlitchPoketch::ActivateMenuGlitchPoketch(const ActivateMenuGlitchPoketch_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , FLY_A_TO_X_DELAY(
        "<b>Fly Menu A-to-X Delay:</b><br>The delay between the A and X presses to overlap the menu with the fly option.",
        "60", 20
    )
{
    PA_ADD_OPTION(FLY_A_TO_X_DELAY);
}



void trigger_menu(ProgramEnvironment& env, ConsoleHandle& console){
    console.botbase().wait_for_all_requests();
    MapDetector detector;
    int ret = run_until(
        env, console,
        [](const BotBaseContext& context){
            for (size_t i = 0; i < 12; i++){
                for (size_t c = 0; c < 42; c++){
                    pbf_controller_state(context, BUTTON_ZL, DPAD_NONE, 128, 128, 128, 128, 1);
                    pbf_controller_state(context, BUTTON_R | BUTTON_ZL, DPAD_NONE, 128, 128, 128, 128, 5);
                    pbf_wait(context, 3);
                }
                pbf_wait(context, 125);
                pbf_press_button(context, BUTTON_R, 20, 105);
            }
        },
        { &detector }
    );
    if (ret < 0){
        console.log("Map not detected after 60 seconds.", Qt::red);
        PA_THROW_StringException("Map not detected after 60 seconds.");
    }
}
void trigger_map_overlap(ProgramEnvironment& env, ConsoleHandle& console){
    for (size_t c = 0; c < 10; c++){
        trigger_menu(env, console);

        pbf_press_dpad(console, DPAD_UP, 50, 0);
        console.botbase().wait_for_all_requests();
        BlackScreenDetector detector;
        int ret = wait_until(
            env, console, std::chrono::seconds(4),
            { &detector }
        );
        if (ret >= 0){
            console.log("Overlap detected! Entered Pokemon center.", Qt::blue);
            return;
        }
        console.log("Failed to activate map overlap.", "orange");
        pbf_mash_button(console, BUTTON_B, 3 * TICKS_PER_SECOND);
        pbf_press_button(console, BUTTON_R, 20, 230);
    }
    console.log("Failed to trigger map overlap after 10 attempts.", Qt::red);
    PA_THROW_StringException("Failed to trigger map overlap after 10 attempts.");
}



void ActivateMenuGlitchPoketch::program(SingleSwitchProgramEnvironment& env){
    ConsoleHandle& console = env.console;

    trigger_map_overlap(env, console);
    pbf_wait(console, 3 * TICKS_PER_SECOND);

    //  Move to escalator.
    pbf_press_dpad(console, DPAD_UP, 20, 105);
    pbf_press_dpad(console, DPAD_UP, 20, 105);
    pbf_move_left_joystick(console, 255, 128, 250, 5 * TICKS_PER_SECOND);

    //  Re-enter escalator.
    pbf_press_dpad(console, DPAD_RIGHT, 125, 5 * TICKS_PER_SECOND);

    //  Leave Pokemon center.
    pbf_press_dpad(console, DPAD_LEFT, 20, 105);
    pbf_press_dpad(console, DPAD_LEFT, 20, 105);
    pbf_press_dpad(console, DPAD_LEFT, 20, 105);
    pbf_press_dpad(console, DPAD_LEFT, 20, 105);
    pbf_press_dpad(console, DPAD_LEFT, 20, 105);
    pbf_move_left_joystick(console, 128, 255, 125, 5 * TICKS_PER_SECOND);

    //  Center cursor.
    pbf_press_button(console, BUTTON_X, 20, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);
    pbf_press_button(console, BUTTON_X, 20, GameSettings::instance().MENU_TO_OVERWORLD_DELAY);

    //  Bring up menu
    pbf_press_button(console, BUTTON_ZL, 20, FLY_A_TO_X_DELAY - 20);
    pbf_press_button(console, BUTTON_X, 20, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);

    //  Fly
    pbf_press_button(console, BUTTON_ZL, 20, 10 * TICKS_PER_SECOND);

    //  Enter Pokemon center.
    pbf_press_dpad(console, DPAD_UP, 50, 5 * TICKS_PER_SECOND);
    pbf_move_left_joystick(console, 255, 128, 125, 0);
    pbf_move_left_joystick(console, 128, 255, 125, 125);

    //  Move cursor back to default location for "Pokemon".
    pbf_move_right_joystick(console, 128, 0, 20, 20);
    pbf_move_right_joystick(console, 0, 128, 20, 20);
    pbf_move_right_joystick(console, 0, 128, 20, 20);

    console.botbase().wait_for_all_requests();
}



}
}
}
