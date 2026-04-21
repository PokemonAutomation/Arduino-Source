/*  Pokemon RSE Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Soft reset, menus, etc.
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Random.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonRSE/Inference/Dialogs/PokemonRSE_DialogDetector.h"
#include "PokemonRSE/Inference/Sounds/PokemonRSE_ShinySoundDetector.h"
#include "PokemonRSE/PokemonRSE_Settings.h"
#include "PokemonRSE_Navigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{


bool try_soft_reset(ConsoleHandle& console, ProControllerContext& context){
    // A + B + Select + Start
    pbf_press_button(context, BUTTON_B | BUTTON_A | BUTTON_MINUS | BUTTON_PLUS, 360ms, 1440ms);

    pbf_mash_button(context, BUTTON_MINUS, GameSettings::instance().SELECT_BUTTON_MASH0);
    context.wait_for_all_requests();

    //Wait for save file select screen
    WhiteScreenOverWatcher whitescreen(COLOR_RED, {0.282, 0.064, 0.448, 0.871});

    int ls = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_PLUS, 1000ms);
            pbf_wait(context, 5000ms);
            context.wait_for_all_requests();
        },
        { whitescreen }
    );
    context.wait_for_all_requests();
    if (ls == 0){
        console.log("Entered load menu.");
    }else{
        console.log("soft_reset(): Unable to enter load menu.", COLOR_RED);
        return false;
    }
    //Let the animations finish
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();

    //Load game
    pbf_press_button(context, BUTTON_A, 160ms, 320ms);

    //Wait for game to load in
    BlackScreenOverWatcher detector2(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    int ret = wait_until(
        console, context,
        GameSettings::instance().ENTER_GAME_WAIT0,
        {detector2}
    );
    if (ret == 0){
        console.log("Entered game!");
    }else{
        console.log("soft_reset(): Timed out waiting to enter game.", COLOR_RED);
        return false;
    }

    //Random wait
    console.log("Randomly waiting to avoid duplicate hits.");
    Milliseconds rng_wait2 = std::chrono::milliseconds(random_u32(0, 5000));
    pbf_wait(context, rng_wait2);
    context.wait_for_all_requests();

    return true;
}

uint64_t soft_reset(ConsoleHandle& console, ProControllerContext& context){
    uint64_t errors = 0;
    for (; errors < 5; errors++){
        if (try_soft_reset(console, context)){
            console.log("Soft reset completed.");
            return errors;
        }
    }
    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "soft_reset(): Failed to reset after 5 attempts.",
        console
    );
}

void flee_battle(VideoStream& stream, ProControllerContext& context){
    AdvanceBattleDialogWatcher ran_away(COLOR_YELLOW);
    int ret2 = run_until<ProControllerContext>(
        stream, context,
        [&](ProControllerContext& context){
            stream.log("Navigate to Run.");
            pbf_press_dpad(context, DPAD_RIGHT, 160ms, 160ms);
            pbf_press_dpad(context, DPAD_DOWN, 160ms, 160ms);
            pbf_press_button(context, BUTTON_A, 160ms, 320ms);
            pbf_wait(context, 5000ms);
            context.wait_for_all_requests();
        },
        { ran_away }
    );
    context.wait_for_all_requests();
    if (ret2 == 0){
        stream.log("Running away...");
    }else{
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "handle_encounter(): Unable to navigate to flee button.",
            stream
        );
    }

    BlackScreenOverWatcher battle_over(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    int ret3 = run_until<ProControllerContext>(
        stream, context,
        [&](ProControllerContext& context){
            pbf_press_button(context, BUTTON_A, 320ms, 320ms);
            pbf_wait(context, 5000ms);
            context.wait_for_all_requests();
        },
        { battle_over }
    );
    context.wait_for_all_requests();
    if (ret3 == 0){
        stream.log("Successfully ran from battle.");
    }else{
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "handle_encounter(): Unable to flee from battle.",
            stream
        );
    }
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
                std::chrono::seconds(30),
                {{legendary_appeared}}
            );
            if (ret == 0){
                console.log("Advance arrow detected.");
            }else{
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "handle_encounter(): Did not detect battle start.",
                    console
                );
            }
            pbf_wait(context, 1000ms);
            context.wait_for_all_requests();
        },
        {{shiny_detector}}
    );
    shiny_detector.throw_if_no_sound(std::chrono::milliseconds(1000));
    if (res == 0){
        console.log("Shiny detected!");
        return true;
    }
    console.log("Shiny not found.");

    if (send_out_lead){
        //Send out lead, no shiny detection needed.
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
        }else{
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

void home_black_border_check(ConsoleHandle& console, ProControllerContext& context){
    if (GameSettings::instance().DEVICE == GameSettings::Device::switch_1_2){
        console.log("Switch 1 or 2 selected in Settings.");

        console.log("Checking for min 720p and 16:9.");
        assert_16_9_720p_min(console, console);

        console.log("Going to home to check for black border.");
        pbf_press_button(context, BUTTON_ZL, 120ms, 880ms); //  Connect the controller.
        pbf_press_button(context, BUTTON_HOME, 120ms, 880ms);
        context.wait_for_all_requests();
        StartProgramChecks::check_border(console);
        console.log("Returning to game.");
        resume_game_from_home(console, context);
        context.wait_for_all_requests();
        console.log("Entered game.");
    }else{
        console.log("Non-Switch device selected in Settings.");
        console.log("Skipping black border and aspect ratio checks.", COLOR_BLUE);
    }
}


}
}
}
