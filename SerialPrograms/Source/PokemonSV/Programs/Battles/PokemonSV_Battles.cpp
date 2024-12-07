/*  Battles
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV_Battles.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


void auto_heal_from_menu_or_overworld(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    uint8_t party_slot,
    bool return_to_overworld
){
    console.log("Auto-healing...");
    WallClock start = current_time();
    bool healed = false;
    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            OperationFailedException::fire(
                console, ErrorReport::SEND_ERROR_REPORT,
                "auto_heal_from_menu(): Failed auto-heal after 5 minutes."
            );
        }

        OverworldWatcher overworld(console, COLOR_RED);
        MainMenuWatcher main_menu(COLOR_CYAN);
        AdvanceDialogWatcher dialog(COLOR_YELLOW);
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context, std::chrono::seconds(60),
            {overworld, main_menu, dialog}
        );
        switch (ret){
        case 0:
            console.log("Detected overworld.");
            if (healed && return_to_overworld){
                return;
            }
            pbf_press_button(context, BUTTON_X, 20, 230);
            continue;
        case 1:
            console.log("Detected main menu.");
            if (!healed){
                main_menu.move_cursor(info, console, context, MenuSide::LEFT, party_slot, false);
                pbf_press_button(context, BUTTON_MINUS, 20, 230);
                healed = true;
                continue;
            }
            if (!return_to_overworld){
                return;
            }
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 2:
            console.log("Detected dialog.");
            healed = true;
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        default:
            OperationFailedException::fire(
                console, ErrorReport::SEND_ERROR_REPORT,
                "auto_heal_from_menu(): No state detected after 60 seconds."
            );
        }
    }
}




int run_from_battle(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    console.log("Attempting to run away...");

    int attempts = 0;
    for (size_t c = 0; c < 10; c++){
        OverworldWatcher overworld(console, COLOR_RED);
        NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
        GradientArrowWatcher next_pokemon(COLOR_GREEN, GradientArrowType::RIGHT, {0.50, 0.51, 0.30, 0.10});
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context, std::chrono::seconds(60),
            {
                overworld,
                battle_menu,
                next_pokemon,
            }
        );

        switch (ret){
        case 0:
            console.log("Detected overworld...");
            return attempts;
        case 1:
            console.log("Detected battle menu...");
            battle_menu.move_to_slot(console, context, 3);
//            pbf_press_dpad(context, DPAD_DOWN, 250, 0);
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_button(context, BUTTON_B, 20, 1 * TICKS_PER_SECOND);
            attempts++;
            continue;
        case 2:
            console.log("Detected own " + STRING_POKEMON + " fainted...");
            OperationFailedException::fire(
                console, ErrorReport::SEND_ERROR_REPORT,
                "Your " + STRING_POKEMON + " fainted while attempting to run away."
            );
        default:
            OperationFailedException::fire(
                console, ErrorReport::SEND_ERROR_REPORT,
                "run_from_battle(): No state detected after 60 seconds."
            );
        }
    }

    OperationFailedException::fire(
        console, ErrorReport::SEND_ERROR_REPORT,
        "Failed to run away after 10 attempts."
    );
}


}
}
}
