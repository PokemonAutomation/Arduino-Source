/*  Synchronized Spinning
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "SynchronizedSpinning.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


SynchronizedSpinning::SynchronizedSpinning()
    : MultiSwitchProgram(
        FeedbackType::NONE, PABotBaseLevel::PABOTBASE_12KB,
        "Synchronized Spinning", "",
        "Don't ask... seriously, don't ask...",
        1, 4, 1
    )
{}

void SynchronizedSpinning::program(MultiSwitchProgramEnvironment& env) const{
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
