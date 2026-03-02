/*  Pokemon FRLG Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Soft reset, menus, etc.
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Random.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/Inference/Sounds/PokemonFRLG_ShinySoundDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_StartMenuDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_LoadMenuDetector.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_Navigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


void soft_reset(ConsoleHandle& console, ProControllerContext& context){
    // A + B + Select + Start
    pbf_press_button(context, BUTTON_B | BUTTON_A | BUTTON_MINUS | BUTTON_PLUS, 360ms, 1440ms);

    pbf_mash_button(context, BUTTON_MINUS, GameSettings::instance().SELECT_BUTTON_MASH0);
    context.wait_for_all_requests();

    //Random wait before pressing start/A
    console.log("Randomly waiting...");
    Milliseconds rng_wait = std::chrono::milliseconds(random_u32(0, 5000));
    pbf_wait(context, rng_wait);
    context.wait_for_all_requests();

    //Mash A until white screen to game load menu
    WhiteScreenOverWatcher whitescreen(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    LoadMenuWatcher load_menu(COLOR_BLUE);

    int ls = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context) {
            pbf_mash_button(context, BUTTON_A, 1000ms);
            pbf_wait(context, 5000ms);
            context.wait_for_all_requests();
        },
        { whitescreen, load_menu }
    );
    context.wait_for_all_requests();
    if (ls == 0) {
        console.log("Entered load menu. (WhiteScreenOver)");
    }else if (ls == 1) {
        console.log("Entered load menu. (LoadMenu)");
    }else{
        console.log("Unable to enter load menu.", COLOR_RED);
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "soft_reset(): Unable to enter load menu.",
            console
        );
    }
    //Let the animation finish
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();

    //Load game
    pbf_press_button(context, BUTTON_A, 160ms, 320ms);

    //Wait for game to load in
    BlackScreenOverWatcher detector(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    int ret = wait_until(
        console, context,
        GameSettings::instance().ENTER_GAME_WAIT0,
        {detector}
    );
    if (ret == 0){
        console.log("Entered game!");
    }else{
        console.log("Timed out waiting to enter game.", COLOR_RED);
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "soft_reset(): Timed out waiting to enter game.",
            console
        );
    }

    //Mash past "previously on..."
    pbf_mash_button(context, BUTTON_B, GameSettings::instance().ENTER_GAME_MASH0);
    context.wait_for_all_requests();

    //Random wait no.2
    console.log("Randomly waiting...");
    Milliseconds rng_wait2 = std::chrono::milliseconds(random_u32(0, 5000));
    pbf_wait(context, rng_wait2);
    context.wait_for_all_requests();

    console.log("Soft reset completed.");
}

void open_slot_six(ConsoleHandle& console, ProControllerContext& context){
    //Attempt to exit any dialog and open the start menu
    StartMenuWatcher start_menu(COLOR_RED);

    int ret = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context){
            for (int i = 0; i < 10; i++){
                pbf_press_button(context, BUTTON_B, 320ms, 640ms);
                pbf_wait(context, 100ms);
                context.wait_for_all_requests();
                pbf_press_button(context, BUTTON_PLUS, 320ms, 640ms);
                pbf_wait(context, 100ms);
                context.wait_for_all_requests();
            }
        },
        { start_menu }
    );
    context.wait_for_all_requests();
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "open_slot_six(): Unable to open Start menu.",
            console
        );
    }

    console.log("Navigating to party menu.");
    BlackScreenOverWatcher blk1(COLOR_RED, {0.282, 0.064, 0.448, 0.871});

    int pm = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context) {
            pbf_wait(context, 200ms);
            context.wait_for_all_requests();
            pbf_press_dpad(context, DPAD_DOWN, 320ms, 320ms);
            context.wait_for_all_requests();

            pbf_press_button(context, BUTTON_A, 320ms, 640ms);
            pbf_wait(context, 5000ms);
            context.wait_for_all_requests();
        },
        { blk1 }
    );
    if (pm == 0){
        console.log("Entered party menu.");
    }else{
        console.log("Unable to enter Party menu.", COLOR_RED);
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "open_slot_six(): Unable to enter Party menu.",
            console
        );
    }
    context.wait_for_all_requests();

    //Press up twice to get to the last slot
    pbf_press_dpad(context, DPAD_UP, 320ms, 320ms);
    pbf_press_dpad(context, DPAD_UP, 320ms, 320ms);

    //Two presses to open summary
    BlackScreenOverWatcher blk2(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    int sm = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context) {
            pbf_press_button(context, BUTTON_A, 320ms, 640ms);
            pbf_press_button(context, BUTTON_A, 320ms, 640ms);
            pbf_wait(context, 5000ms);
            context.wait_for_all_requests();
        },
        { blk2 }
    );
    if (sm == 0){
        console.log("Entered summary.");
    }else{
        console.log("Unable to enter summary.", COLOR_RED);
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "open_slot_six(): Unable to enter summary.",
            console
        );
    }
    pbf_wait(context, 1000ms);
    context.wait_for_all_requests();
}

bool handle_encounter(ConsoleHandle& console, ProControllerContext& context, bool send_out_lead){
    float shiny_coefficient = 1.0;
    ShinySoundDetector shiny_detector(console.logger(), [&](float error_coefficient) -> bool{
        shiny_coefficient = error_coefficient;
        return true;
    });
    AdvanceBattleDialogWatcher legendary_appeared(COLOR_YELLOW);

    int res = run_until<ProControllerContext>(
        console, context,
        [&](ProControllerContext& context){
            int ret = wait_until(
                console, context,
                std::chrono::seconds(30), //More than enough time for shiny sound
                {{legendary_appeared}}
            );
            if (ret == 0){
                console.log("Battle Advance arrow detected.");
            }else{
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "handle_encounter(): Did not detect battle advance arrow.",
                    console
                );
            }
            pbf_wait(context, 1000ms);
            context.wait_for_all_requests();

            /* 
            //Send out shiny lead to test detection
            BattleMenuWatcher battle_menu(COLOR_RED);
            console.log("Sending out lead Pokemon.");
            pbf_press_button(context, BUTTON_A, 320ms, 320ms);

            int ret2 = wait_until(
                console, context,
                std::chrono::seconds(15),
                { {battle_menu} }
            );
            if (ret2 == 0){
                console.log("Battle menu detecteed!");
            }
            else {
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "handle_encounter(): Did not detect battle menu.",
                    console
                );
            }
            pbf_wait(context, 100000ms); //extreme audio delay on my cheap test device
            context.wait_for_all_requests();
            */

        },
        {{shiny_detector}}
    );
    shiny_detector.throw_if_no_sound();
    if (res == 0){
        console.log("Shiny detected!");
        return true;
    }
    console.log("No shiny detected.");

    if (send_out_lead){
        //Send out lead, no shiny detection needed. (Or wanted.)
        BattleMenuWatcher battle_menu(COLOR_RED);
        console.log("Sending out lead Pokemon.");
        pbf_press_button(context, BUTTON_A, 320ms, 320ms);

        int ret = wait_until(
            console, context,
            std::chrono::seconds(15),
            { {battle_menu} }
        );
        if (ret == 0){
            console.log("Battle menu detecteed!");
        }
        else {
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "handle_encounter(): Did not detect battle menu.",
                console
            );
        }
        pbf_wait(context, 1000ms);
        context.wait_for_all_requests();
    }

    return false;
}

void flee_battle(ConsoleHandle& console, ProControllerContext& context){
    console.log("Navigate to Run.");
    pbf_press_dpad(context, DPAD_RIGHT, 160ms, 160ms);
    pbf_press_dpad(context, DPAD_DOWN, 160ms, 160ms);
    pbf_press_button(context, BUTTON_A, 160ms, 320ms);

    AdvanceBattleDialogWatcher ran_away(COLOR_YELLOW);
    int ret2 = wait_until(
        console, context,
        std::chrono::seconds(5),
        {{ran_away}}
    );
    if (ret2 == 0){
        console.log("Running away...");
    }else{
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "flee_battle(): Unable to navigate to flee button.",
            console
        );
    }

    pbf_press_button(context, BUTTON_A, 320ms, 320ms);
    BlackScreenOverWatcher battle_over(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    int ret3 = wait_until(
        console, context,
        std::chrono::seconds(5),
        {{battle_over}}
    );
    if (ret3 == 0){
        console.log("Successfully fled the battle.");
    }else{
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "flee_battle(): Unable to flee from battle.",
            console
        );
    }
}


}
}
}
