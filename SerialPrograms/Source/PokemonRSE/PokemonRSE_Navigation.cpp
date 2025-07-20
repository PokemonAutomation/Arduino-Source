/*  Pokemon RSE Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Soft reset, menus, etc.
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonRSE/Inference/Dialogs/PokemonRSE_DialogDetector.h"
#include "PokemonRSE/Inference/Sounds/PokemonRSE_ShinySoundDetector.h"
#include "PokemonRSE/PokemonRSE_Settings.h"
#include "PokemonRSE_Navigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{


void soft_reset(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    // A + B + Select + Start
    pbf_press_button(context, BUTTON_B | BUTTON_Y | BUTTON_MINUS | BUTTON_PLUS, 10, 180);

    pbf_mash_button(context, BUTTON_PLUS, GameSettings::instance().START_BUTTON_MASH0);
    context.wait_for_all_requests();

    pbf_press_button(context, BUTTON_A, 20, 40);

    //Wait for game to load in
    BlackScreenOverWatcher detector(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    int ret = wait_until(
        stream, context,
        GameSettings::instance().ENTER_GAME_WAIT0,
        {detector}
    );
    if (ret == 0){
        stream.log("Entered game!");
    }else{
        stream.log("Timed out waiting to enter game.", COLOR_RED);
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "soft_reset(): Timed out waiting to enter game.",
            stream
        );
    }
    context.wait_for_all_requests();
}

void flee_battle(VideoStream& stream, ProControllerContext& context) {
    stream.log("Navigate to Run.");
    pbf_press_dpad(context, DPAD_RIGHT, 20, 20);
    pbf_press_dpad(context, DPAD_DOWN, 20, 20);
    pbf_press_button(context, BUTTON_A, 20, 40);

    AdvanceBattleDialogWatcher ran_away(COLOR_YELLOW);
    int ret2 = wait_until(
        stream, context,
        std::chrono::seconds(5),
        {{ran_away}}
    );
    if (ret2 == 0) {
        stream.log("Running away...");
    } else {
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "handle_encounter(): Unable to navigate to flee button.",
            stream
        );
    }

    pbf_press_button(context, BUTTON_A, 40, 40);
    BlackScreenOverWatcher battle_over(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    int ret3 = wait_until(
        stream, context,
        std::chrono::seconds(5),
        {{battle_over}}
    );
    if (ret3 == 0) {
        stream.log("Ran from battle.");
    } else {
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "handle_encounter(): Unable to flee from battle.",
            stream
        );
    }
}

bool handle_encounter(VideoStream& stream, ProControllerContext& context, bool send_out_lead) {
    float shiny_coefficient = 1.0;
    ShinySoundDetector shiny_detector(stream.logger(), [&](float error_coefficient) -> bool{
        shiny_coefficient = error_coefficient;
        return true;
    });
    AdvanceBattleDialogWatcher legendary_appeared(COLOR_YELLOW);

    stream.log("Starting battle.");
    pbf_mash_button(context, BUTTON_A, 540);
    context.wait_for_all_requests();

    int res = run_until<ProControllerContext>(
        stream, context,
        [&](ProControllerContext& context) {
            int ret = wait_until(
                stream, context,
                std::chrono::seconds(30),
                {{legendary_appeared}}
            );
            if (ret == 0) {
                stream.log("Advance arrow detected.");
            } else {
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "handle_encounter(): Did not detect battle start.",
                    stream
                );
            }
            pbf_wait(context, 125);
            context.wait_for_all_requests();
        },
        {{shiny_detector}}
    );
    shiny_detector.throw_if_no_sound();
    if (res == 0){
        stream.log("Shiny detected!");
        return true;
    }
    stream.log("Shiny not found.");

    if (send_out_lead) {
        //Send out lead, no shiny detection needed.
        BattleMenuWatcher battle_menu(COLOR_RED);
        stream.log("Sending out lead Pokemon.");
        pbf_press_button(context, BUTTON_A, 40, 40);

        int ret = wait_until(
            stream, context,
            std::chrono::seconds(15),
            { {battle_menu} }
        );
        if (ret == 0) {
            stream.log("Battle menu detecteed!");
        }
        else {
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "handle_encounter(): Did not detect battle menu.",
                stream
            );
        }
        pbf_wait(context, 125);
        context.wait_for_all_requests();
    }

    return false;
}


}
}
}
