/*  Heal the party using Global Room
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h"
#include "PokemonBDSP_GlobalRoomHeal.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



bool heal_by_global_room(ProgramEnvironment& env, ConsoleHandle& console){
    // Go to union room menu.
    const uint16_t overworld_to_room_delay = 125;
    pbf_press_button(console, BUTTON_Y, 10, overworld_to_room_delay);

    // Go to global room.
    pbf_press_dpad(console, DPAD_RIGHT, 10, 100);

    // Press ZL until we are at:
    // - "Would you like to enter the Global Room?" To select: "Yes" and other options.
    SelectionArrowFinder arrow(console, {0.50, 0.45, 0.20, 0.20}, COLOR_GREEN);
    int ret = run_until(
        env, console,
        [=](const BotBaseContext& context){
            for (int i = 0; i < 5; i++){
                pbf_press_button(context, BUTTON_ZL, 10, 125);
            }
        },
        { &arrow }
    );
    if (ret < 0){
        throw OperationFailedException(console, "No selection arrow detected when using Global Room.");
    }

    // Select "Yes"
    pbf_press_button(console, BUTTON_ZL, 10, 125);
    
    // Then mash B to leave Union Room
    pbf_mash_button(console, BUTTON_B, 400);

    console.botbase().wait_for_all_requests();
    return true;
}



}
}
}
