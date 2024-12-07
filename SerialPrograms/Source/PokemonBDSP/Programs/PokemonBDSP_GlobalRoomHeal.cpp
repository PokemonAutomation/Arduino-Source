/*  Heal the party using Global Room
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h"
#include "PokemonBDSP_GlobalRoomHeal.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



bool heal_by_global_room(ConsoleHandle& console, BotBaseContext& context){
    console.overlay().add_log("Heal by Global Room", COLOR_WHITE);
    // Go to union room menu.
    const uint16_t overworld_to_room_delay = 125;
    pbf_press_button(context, BUTTON_Y, 10, overworld_to_room_delay);

    // Go to global room.
    pbf_press_dpad(context, DPAD_RIGHT, 10, 100);

    // Press ZL until we are at:
    // - "Would you like to enter the Global Room?" To select: "Yes" and other options.
    SelectionArrowFinder arrow(console, {0.50, 0.45, 0.20, 0.20}, COLOR_GREEN);
    int ret = run_until(
        console, context,
        [](BotBaseContext& context){
            for (int i = 0; i < 5; i++){
                pbf_press_button(context, BUTTON_ZL, 10, 125);
            }
        },
        {{arrow}}
    );
    if (ret < 0){
        OperationFailedException::fire(
            console, ErrorReport::SEND_ERROR_REPORT,
            "No selection arrow detected when using Global Room."
        );
    }

    // Select "Yes"
    pbf_press_button(context, BUTTON_ZL, 10, 125);
    
    // Then mash B to leave Union Room
    pbf_mash_button(context, BUTTON_B, 400);

    context.wait_for_all_requests();
    console.overlay().add_log("Heal complete", COLOR_WHITE);
    return true;
}



}
}
}
