/*  Turbo A
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

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
        "ComputerControl/blob/master/Wiki/Programs/NintendoSwitch/TurboA.md",
        "Endlessly mash A.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}



TurboA::TurboA(){
    PA_ADD_OPTION(START_LOCATION);
}
void TurboA::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        PokemonSwSh::resume_game_back_out(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    }

    while (true){
        ssf_mash1_button(context, BUTTON_A, 10000ms);
    }
}




}
}

