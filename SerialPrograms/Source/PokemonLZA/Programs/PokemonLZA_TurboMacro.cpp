/*  Turbo Macro
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "CommonTools/Async/InferenceRoutines.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLZA/Options/PokemonLZA_ShinyDetectedAction.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonLZA_TurboMacro.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


LZA_TurboMacro_Descriptor::LZA_TurboMacro_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:TurboMacro",
        "Nintendo Switch", "Turbo Macro",
        "Programs/NintendoSwitch/TurboMacro.html",
        "Create macros",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

LZA_TurboMacro::LZA_TurboMacro()
    : LOOP(
        "<b>Number of times to loop:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        100, 0
    )
    , TABLE(
        "Command Schedule:",
        {
            ControllerClass::NintendoSwitch_ProController,
            ControllerClass::NintendoSwitch_LeftJoycon,
            ControllerClass::NintendoSwitch_RightJoycon,
        }
    )
    , GO_HOME_WHEN_DONE(true)
    , RUN_UNTIL_CALLBACK(
        "Trigger to stop program:",
        {
            {RunUntilCallback::NONE,         "none",           "None (stop when done all loops)"},
            {RunUntilCallback::SHINY_SOUND,            "shiny-sound",              "Shiny Sound Detected"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        RunUntilCallback::NONE
    )
{
    PA_ADD_OPTION(LOOP);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(RUN_UNTIL_CALLBACK);
    PA_ADD_OPTION(TABLE);
}


void LZA_TurboMacro::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    ProControllerContext context(scope, env.console.controller<ProController>());

    switch (RUN_UNTIL_CALLBACK){
    case RunUntilCallback::NONE:
        run_table(env, scope);
        break;
    case RunUntilCallback::SHINY_SOUND:
        run_table_stop_when_shiny_sound(env, scope);
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "TurboMacro::program(): Unknown RunUntilCallback");
        
    }

    if (GO_HOME_WHEN_DONE){
        go_home(env.console, context);
    }
}


void LZA_TurboMacro::run_table(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    for (uint32_t c = 0; c < LOOP; c++){
        TABLE.run(scope, env.console.controller());
    }
}

void LZA_TurboMacro::run_table_stop_when_shiny_sound(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    ShinySoundDetectedActionOption shiny_detected_option("Shiny Detected", "", "1000 ms", ShinySoundDetectedAction::NOTIFY_ON_FIRST_ONLY);
    ShinySoundHandler shiny_sound_handler(shiny_detected_option);
    PokemonLA::ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool {
        //  Warning: This callback will be run from a different thread than this function.
        // env.console.overlay().add_log("Shiny Sound Detected!", COLOR_YELLOW);
        return shiny_sound_handler.on_shiny_sound(
            env, env.console,
            0,
            error_coefficient
        );
    });

    int ret = run_until(
        env.console, scope,
        [&](CancellableScope& scope){
            run_table(env, scope);
        },
        {shiny_detector}
    );

    if (ret == 0){
        return;
    }
}

}
}
}
