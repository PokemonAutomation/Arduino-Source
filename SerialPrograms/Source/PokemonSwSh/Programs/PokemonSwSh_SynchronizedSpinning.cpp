/*  Synchronized Spinning
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh_SynchronizedSpinning.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


SynchronizedSpinning_Descriptor::SynchronizedSpinning_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonSwSh:SynchronizedSpinning",
        STRING_POKEMON + " SwSh", "Synchronized Spinning", "",
        "Don't ask... seriously, don't ask...",
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        1, 4, 1
    )
{}



SynchronizedSpinning::SynchronizedSpinning(){}

void SynchronizedSpinning::program(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
    env.run_in_parallel(
        scope,
        [&](ConsoleHandle& console, ProControllerContext& context){
            pbf_move_left_joystick(context, 128, 255, 5, 20);
            while (true){
                pbf_move_left_joystick(context, 128, 0, 5, 0);
                pbf_move_left_joystick(context, 255, 0, 5, 0);
                pbf_move_left_joystick(context, 255, 128, 5, 0);
                pbf_move_left_joystick(context, 255, 255, 5, 0);
                pbf_move_left_joystick(context, 128, 255, 5, 0);
                pbf_move_left_joystick(context, 0, 255, 5, 0);
                pbf_move_left_joystick(context, 0, 128, 5, 0);
                pbf_move_left_joystick(context, 0, 0, 5, 0);
            }
        }
    );
}



}
}
}
