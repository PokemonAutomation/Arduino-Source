/*  Start Menu Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Time.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_StartMenuDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_SelectionArrowDetector.h"
#include "PokemonFRLG_StartMenuNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG {

void save_game_to_overworld(ConsoleHandle& console, ProControllerContext& context){

    bool seen_start_menu = false;

    WallClock start = current_time();
    while (true){
        context.wait_for_all_requests();

        if (current_time() - start > std::chrono::seconds(120)) {
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "save_game_to_overworld(): Unable to save game after 2 minutes.",
                console
            );
        }

        StartMenuWatcher start_menu = StartMenuWatcher(COLOR_RED);

        int ret = wait_until(
            console, context,
            std::chrono::seconds(1),
            {
                start_menu
            }
        );

        if (ret == 0) {
            seen_start_menu = true;
        }

        while (!seen_start_menu)
        {
            int ret2 = run_until<ProControllerContext>(
                console, context,
                [](ProControllerContext& context) {
                    pbf_press_button(context, BUTTON_PLUS, 320ms, 200ms);
                    pbf_wait(context, 100ms);
                    context.wait_for_all_requests();
                },
                { start_menu }
            );

            if (ret2 == 0) {
                seen_start_menu = true;
            }
        }

        SelectionArrowWatcher pokedex_arrow = SelectionArrowWatcher(
            COLOR_RED,
            &console.overlay(),
            SelectionArrowPosition::START_MENU_POKEDEX
        );

        SelectionArrowWatcher pokemon_arrow = SelectionArrowWatcher(
            COLOR_RED,
            &console.overlay(),
            SelectionArrowPosition::START_MENU_POKEMON
        );

        SelectionArrowWatcher bag_arrow = SelectionArrowWatcher(
            COLOR_RED,
            &console.overlay(),
            SelectionArrowPosition::START_MENU_BAG
        );

        SelectionArrowWatcher trainer_arrow = SelectionArrowWatcher(
            COLOR_RED,
            &console.overlay(),
            SelectionArrowPosition::START_MENU_TRAINER
        );

        SelectionArrowWatcher save_arrow = SelectionArrowWatcher(
            COLOR_RED,
            &console.overlay(),
            SelectionArrowPosition::START_MENU_SAVE
        );

        SelectionArrowWatcher option_arrow = SelectionArrowWatcher(
            COLOR_RED,
            &console.overlay(),
            SelectionArrowPosition::START_MENU_OPTION
        );

        SelectionArrowWatcher exit_arrow = SelectionArrowWatcher(
            COLOR_RED,
            &console.overlay(),
            SelectionArrowPosition::START_MENU_EXIT
        );

        int ret3 = wait_until(
            console, context,
            std::chrono::seconds(2),
            {
                pokedex_arrow,
                pokemon_arrow,
                bag_arrow,
                trainer_arrow,
                save_arrow,
                option_arrow,
                exit_arrow
            }
        );

        switch (ret3){
        case 0:
            console.log("Detected Pokedex Arrow. Naviating to 'SAVE'.");
            pbf_press_dpad(context, DPAD_UP, 320ms, 400ms);
            pbf_press_dpad(context, DPAD_UP, 320ms, 400ms);
            pbf_press_dpad(context, DPAD_UP, 320ms, 400ms);
            context.wait_for_all_requests();
            break;
        case 1:
            console.log("Detected Pokemon Arrow. Naviating to 'SAVE'.");
            pbf_press_dpad(context, DPAD_DOWN, 320ms, 400ms);
            pbf_press_dpad(context, DPAD_DOWN, 320ms, 400ms);
            pbf_press_dpad(context, DPAD_DOWN, 320ms, 400ms);
            context.wait_for_all_requests();
            break;
        case 2:
            console.log("Detected Bag Arrow. Naviating to 'SAVE'.");
            pbf_press_dpad(context, DPAD_DOWN, 320ms, 400ms);
            pbf_press_dpad(context, DPAD_DOWN, 320ms, 400ms);
            context.wait_for_all_requests();
            break;
        case 3:
            console.log("Detected Trainer Arrow. Naviating to 'SAVE'.");
            pbf_press_dpad(context, DPAD_DOWN, 320ms, 400ms);
            context.wait_for_all_requests();
            break;
        case 4:
            console.log("Detected Save Arrow.");
            break;
        case 5:
            console.log("Detected Option Arrow. Naviating to 'SAVE'.");
            pbf_press_dpad(context, DPAD_UP, 320ms, 400ms);
            context.wait_for_all_requests();
            break;
        case 6:
            console.log("Detected Exit Arrow. Naviating to 'SAVE'.");
            pbf_press_dpad(context, DPAD_UP, 320ms, 400ms);
            pbf_press_dpad(context, DPAD_UP, 320ms, 400ms);
            context.wait_for_all_requests();
            break;
        default:
            continue;
        }

        pbf_press_button(context, BUTTON_A, 320ms, 400ms);

        bool save_confirmed = false;
        // There can be one or two confirmation dialogs depending on whether the game was saved prior to this.
        while (true){
            if (current_time() - start > std::chrono::seconds(120)) {
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "save_game_to_overworld(): Unable to save game after 2 minutes.",
                    console
                );
            }

            SelectionArrowWatcher save_confirm_arrow = SelectionArrowWatcher(
                COLOR_RED,
                &console.overlay(),
                SelectionArrowPosition::CHOICE_MENU_YES
            );

            int ret4 = wait_until(
                console, context,
                std::chrono::seconds(10),
                {
                    save_confirm_arrow
                }
            );

            if (ret4 == 0) {
                console.log("Detected Save Confirmation Arrow. Saving game.");
                pbf_press_button(context, BUTTON_A, 320ms, 200ms);
                save_confirmed = true;
            }

            if (ret4 != 0 && save_confirmed) {
                break;
            }

            context.wait_for_all_requests();
        }

        context.wait_for_all_requests();

        return;
    }

}

}
}
}