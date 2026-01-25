/*  Turbo A
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Time.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "NintendoSwitch_TurboA.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


TurboA_Descriptor::TurboA_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:TurboA",
        "Nintendo Switch", "Turbo A",
        "Programs/NintendoSwitch/TurboA.html",
        "Endlessly mash A.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}



TurboA::TurboA()
    : TIME_LIMIT(
        "<b>Time Limit:</b><br>Stop mashing A after this amount of time. Set to 0 for no time limit.",
        LockMode::LOCK_WHILE_RUNNING,
        "0 s"
    )
    , GO_HOME_WHEN_DONE(false)
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(TIME_LIMIT);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
}
void TurboA::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        PokemonSwSh::resume_game_back_out(
            env.console,
            context,
            ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST,
            1600ms
        );
    }

    const std::chrono::milliseconds time_limit = TIME_LIMIT;
    const uint64_t total_ms = time_limit.count();
    if (total_ms == 0){
        while (true){
            ssf_mash1_button(context, BUTTON_A, 10000ms);
        }
        // will never return
    }

    // Mash until time limit is reached
    const uint64_t num_iters = total_ms / 1000;
    const uint64_t remaining_ms = total_ms % 1000;
    for(uint64_t i = 0; i < num_iters; i++){
        ssf_mash1_button(context, BUTTON_A, 1000ms);
    }
    if (remaining_ms > 0){
        ssf_mash1_button(context, BUTTON_A, Milliseconds(remaining_ms));
    }
    context.wait_for_all_requests();
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}




}
}

