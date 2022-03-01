/*  Activate Menu Glitch (1.1.3)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MapDetector.h"
#include "PokemonBDSP_ActivateMenuGlitch-1.1.3.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


ActivateMenuGlitch113_Descriptor::ActivateMenuGlitch113_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonBDSP:ActivateMenuGlitch113",
        STRING_POKEMON + " BDSP", "Activate Menu Glitch (1.1.3)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/ActivateMenuGlitch-113.md",
        "Activate the menu glitch using the strength/fly method. "
        "<font color=\"red\">(This works on game versions 1.1.1 - 1.1.3. It will likely be patched in the future.)</font>",
        FeedbackType::OPTIONAL_, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

ActivateMenuGlitch113::ActivateMenuGlitch113(const ActivateMenuGlitch113_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , FLY_A_TO_X_DELAY(
        "<b>Fly Menu A-to-X Delay:</b><br>The delay between the A and X presses to overlap the menu with the fly option.<br>"
        "(German players may need to increase this to 90.)",
        "50", 20
    )
{
    PA_ADD_OPTION(FLY_A_TO_X_DELAY);
}



void ActivateMenuGlitch113::program(SingleSwitchProgramEnvironment& env){
    ConsoleHandle& console = env.console;

    //  Enable Strength
    pbf_mash_button(console, BUTTON_ZL, 2 * TICKS_PER_SECOND);
    pbf_mash_button(console, BUTTON_B, 5 * TICKS_PER_SECOND);


    pbf_press_button(console, BUTTON_R, 5, 0);
    pbf_press_dpad(console, DPAD_RIGHT, 10, 115);
    pbf_press_button(console, BUTTON_ZL, 10, 0);
    console.botbase().wait_for_all_requests();
    MapWatcher detector;
    int ret = wait_until(
        env, console, std::chrono::seconds(2),
        { &detector }
    );
    if (ret < 0){
        throw OperationFailedException(console, "Map not detected after 2 seconds.");
    }else{
        console.log("Detected map!", COLOR_BLUE);
    }

    env.wait_for(std::chrono::seconds(1));

    //  Move bolder and cursor to Celestial town.
    pbf_press_dpad(console, DPAD_RIGHT, 30, 95);

    //  Bring up menu
    pbf_press_button(console, BUTTON_ZL, 20, FLY_A_TO_X_DELAY - 20);
    pbf_press_button(console, BUTTON_X, 20, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);

    //  Fly
    pbf_press_button(console, BUTTON_ZL, 20, 10 * TICKS_PER_SECOND);
}


}
}
}
