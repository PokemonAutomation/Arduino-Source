/*  Synchronized Spinning
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh_SynchronizedSpinning.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


SynchronizedSpinning_Descriptor::SynchronizedSpinning_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonSwSh:SynchronizedSpinning",
        STRING_POKEMON + " SwSh", "Synchronized Spinning", "",
        "Don't ask... seriously, don't ask...",
        FeedbackType::NONE,
        PABotBaseLevel::PABOTBASE_12KB,
        1, 4, 1
    )
{}



SynchronizedSpinning::SynchronizedSpinning(const SynchronizedSpinning_Descriptor& description)
    : MultiSwitchProgramInstance(description)
{}

void SynchronizedSpinning::program(MultiSwitchProgramEnvironment& env){
    env.run_in_parallel(
        [&](ConsoleHandle& console){
            pbf_move_left_joystick(console, 128, 255, 5, 20);
            while (true){
                pbf_move_left_joystick(console, 128, 0, 5, 0);
                pbf_move_left_joystick(console, 255, 0, 5, 0);
                pbf_move_left_joystick(console, 255, 128, 5, 0);
                pbf_move_left_joystick(console, 255, 255, 5, 0);
                pbf_move_left_joystick(console, 128, 255, 5, 0);
                pbf_move_left_joystick(console, 0, 255, 5, 0);
                pbf_move_left_joystick(console, 0, 128, 5, 0);
                pbf_move_left_joystick(console, 0, 0, 5, 0);
            }
        }
    );
}



}
}
}
