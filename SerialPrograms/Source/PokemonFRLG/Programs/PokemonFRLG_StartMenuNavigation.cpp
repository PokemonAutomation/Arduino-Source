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

bool move_cursor_to_position(ConsoleHandle& console, ProControllerContext& context, SelectionArrowPositionStartMenu destination){
    SelectionArrowWatcher pokedex_arrow = SelectionArrowWatcher(
        COLOR_RED,
        &console.overlay(),
        SelectionArrowPositionStartMenu::POKEDEX
    );

    SelectionArrowWatcher pokemon_arrow = SelectionArrowWatcher(
        COLOR_RED,
        &console.overlay(),
        SelectionArrowPositionStartMenu::POKEMON
    );

    SelectionArrowWatcher bag_arrow = SelectionArrowWatcher(
        COLOR_RED,
        &console.overlay(),
        SelectionArrowPositionStartMenu::BAG
    );

    SelectionArrowWatcher trainer_arrow = SelectionArrowWatcher(
        COLOR_RED,
        &console.overlay(),
        SelectionArrowPositionStartMenu::TRAINER
    );

    SelectionArrowWatcher save_arrow = SelectionArrowWatcher(
        COLOR_RED,
        &console.overlay(),
        SelectionArrowPositionStartMenu::SAVE
    );

    SelectionArrowWatcher option_arrow = SelectionArrowWatcher(
        COLOR_RED,
        &console.overlay(),
        SelectionArrowPositionStartMenu::OPTION
    );

    SelectionArrowWatcher exit_arrow = SelectionArrowWatcher(
        COLOR_RED,
        &console.overlay(),
        SelectionArrowPositionStartMenu::EXIT
    );

    // The order of these watchers needs to match the order of the SelectionArrowPositionStartMenu enum for the math below to work.
    int ret = wait_until(
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

    if (ret < 0) {
        console.log("Unable to detect selection arrow. Not moving cursor.", COLOR_RED);
        return false;
    }

    int destination_index = static_cast<int>(destination);
    int forward = (destination_index - ret + START_MENU_OPTION_COUNT) % START_MENU_OPTION_COUNT;
    int backward = (ret - destination_index + START_MENU_OPTION_COUNT) % START_MENU_OPTION_COUNT;
    if (forward <= backward) {
        for (int i = 0; i < forward; i++) {
            pbf_press_dpad(context, DPAD_DOWN, 320ms, 400ms);
        }
        context.wait_for_all_requests();
    }
    else {
        for (int i = 0; i < backward; i++)
        {
            pbf_press_dpad(context, DPAD_UP, 320ms, 400ms);
        }
        context.wait_for_all_requests();
    }

    return true;
}

void save_game_to_overworld(ConsoleHandle& console, ProControllerContext& context){

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

        if (ret != 0) {
            console.log("Start menu not detected. Attempting to open start menu.");
            pbf_press_button(context, BUTTON_X, 320ms, 320ms);
            context.wait_for_all_requests();
            continue;
        }

        if (!move_cursor_to_position(console, context, SelectionArrowPositionStartMenu::SAVE)) {
            console.log("Unable to detect selection arrow. Attempting to open start menu again.");
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
                SelectionArrowPositionConfirmationMenu::YES
            );

            int ret4 = wait_until(
                console, context,
                std::chrono::seconds(1),
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