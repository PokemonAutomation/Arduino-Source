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
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_BattleDialogs.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_PartyDialogs.h"
#include "PokemonFRLG/Inference/Sounds/PokemonFRLG_ShinySoundDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_StartMenuDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_LoadMenuDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_SummaryDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_PartyMenuDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_BagDetector.h"
#include "PokemonFRLG/Inference/Map/PokemonFRLG_MapDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_BattlePokemonDetector.h"
#include "PokemonFRLG/Programs/PokemonFRLG_StartMenuNavigation.h"
#include "PokemonFRLG_Navigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


bool try_soft_reset(ConsoleHandle& console, ProControllerContext& context){
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
    WhiteScreenOverWatcher whitescreen(COLOR_RED);
    LoadMenuWatcher load_menu(COLOR_BLUE);

    int ls = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, 1000ms);
            pbf_wait(context, 5000ms);
            context.wait_for_all_requests();
        },
        { whitescreen, load_menu }
    );
    context.wait_for_all_requests();
    if (ls == 0){
        console.log("Entered load menu. (WhiteScreenOver)");
    }else if (ls == 1){
        console.log("Entered load menu. (LoadMenu)");
    }else{
        console.log("soft_reset(): Unable to enter load menu.", COLOR_RED);
        return false;
    }
    //Let the animation finish
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();

    //Load game
    pbf_press_button(context, BUTTON_A, 160ms, 320ms);

    //Wait for game to load in
    BlackScreenOverWatcher detector(COLOR_RED);
    int ret = wait_until(
        console, context,
        GameSettings::instance().ENTER_GAME_WAIT0,
        {detector}
    );
    if (ret == 0){
        console.log("Entered game!");
    }else{
        console.log("soft_reset(): Timed out waiting to enter game.", COLOR_RED);
        return false;
    }

    //Mash past "previously on..."
    pbf_mash_button(context, BUTTON_B, GameSettings::instance().ENTER_GAME_MASH0);
    context.wait_for_all_requests();

    //Random wait no.2
    console.log("Randomly waiting...");
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

bool try_open_slot_six(ConsoleHandle& console, ProControllerContext& context){
    //  Attempt to exit any dialog and open the start menu
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
        console.log("open_slot_six(): Unable to open Start menu.", COLOR_RED);
        return false;
    }

    if (!move_cursor_to_position(console, context, SelectionArrowPositionStartMenu::POKEMON)){
        console.log("open_slot_six(): Unable to move menu cursor to: " + Pokemon::STRING_POKEMON, COLOR_RED);
        return false;
    }

    console.log("Navigating to party menu.");
    PartyMenuWatcher blk1(COLOR_RED);

    int pm = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context){
            pbf_press_button(context, BUTTON_A, 320ms, 5640ms);
            context.wait_for_all_requests();
        },
        { blk1 }
    );
    if (pm == 0){
        console.log("Entered party menu.");
    }else{
        console.log("open_slot_six(): Unable to enter Party menu.", COLOR_RED);
        return false;
    }
    context.wait_for_all_requests();

    //Press up twice to get to the last slot
    PartySlotWatcher last_slot(COLOR_RED, PartySlot::SIX);
    int ps = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context){
            for (int i = 0; i < 15; i++){ //Enough to cycle through 6pty+cxl twice
                pbf_wait(context, 320ms);
                context.wait_for_all_requests();
                pbf_press_dpad(context, DPAD_UP, 320ms, 320ms);
            }
        },
        { last_slot }
        );
    context.wait_for_all_requests();
    if (ps == 0){
        console.log("Moved selection to slot six.");
    }else{
        console.log("open_slot_six(): Unable to move selection to slot six.", COLOR_RED);
        return false;
    }

    //Two presses to open summary
    BlackScreenOverWatcher blk2(COLOR_RED);
    int sm = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context){
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
        console.log("open_slot_six(): Unable to enter summary.", COLOR_RED);
        return false;
    }

    //Double check that we are on summary
    SummaryWatcher sum1(COLOR_RED);
    int sm1 = wait_until(
        console, context,
        std::chrono::seconds(5),
        {{ sum1 }}
    );
    if (sm1 == 0){
        console.log("Summary page dots detected.");
    }else{
        console.log("open_slot_six(): Unable to detect summary screen.", COLOR_RED);
        return false;
    }

    pbf_wait(context, 1000ms);
    context.wait_for_all_requests();
    return true;
}

uint64_t open_slot_six(ConsoleHandle& console, ProControllerContext& context){
    uint64_t errors = 0;
    for (; errors < 5; errors++){
        if (try_open_slot_six(console, context)){
            return errors;
        }else{
            console.log("Mashing B to return to overworld and retry...");
            pbf_mash_button(context, BUTTON_B, 10000ms);
        }
    }
    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "open_slot_six(): Failed to open party summary after 5 attempts.",
        console
    );
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
            }else{
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
    shiny_detector.throw_if_no_sound(std::chrono::milliseconds(1000));
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

BattleResult spam_first_move(ConsoleHandle& console, ProControllerContext& context){
    uint16_t errors = 0;
    uint16_t times_moved = 0;
    while (true){
        if (errors > 5) {
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "spam_first_move(): Failed to use move 5 times.",
                console
            );  
        } else if (times_moved > 50){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "spam_first_move(): More than 50 move uses detected.",
                console
            );  
        }

        BattleMenuWatcher battle_menu(COLOR_RED);
        BattleFaintWatcher pokemon_fainted(COLOR_RED);
        BattleOpponentFaintWatcher opponent_fainted(COLOR_RED);
        BlackScreenWatcher battle_ended(COLOR_RED);
        BattleOutOfPpWatcher out_of_pp(COLOR_RED);
        AdvanceBattleDialogWatcher out_of_pp_dialog(COLOR_RED);

        int ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context){
                pbf_wait(context, 20000ms);
                context.wait_for_all_requests();
            },
            { battle_menu, pokemon_fainted, opponent_fainted, battle_ended }
        );

        int ret2;
        switch (ret){
        case 0:
            console.log("Using first move.");
            context.wait_for_all_requests();
            ret2 = run_until<ProControllerContext>(
                console, context,
                [](ProControllerContext& context){
                    pbf_press_button(context, BUTTON_A, 200ms, 300ms); // leave enough time for PP color to be detected
                    pbf_press_button(context, BUTTON_A, 200ms, 300ms);
                    pbf_mash_button(context, BUTTON_A, 500ms);
                    context.wait_for_all_requests();
                },
                { out_of_pp, out_of_pp_dialog }
            );
            if (ret2 < 0){
                times_moved++;
            } else {
                console.log("Out of PP, fleeing battle.");
                pbf_mash_button(context, BUTTON_B, 2000ms);
                flee_battle(console, context);
                context.wait_for_all_requests();
                return BattleResult::outofpp;
            }
            continue;
        case 1:
            console.log("Player Pokemon fainted.");
            return BattleResult::playerfainted;
        case 2:
            console.log("Opponent fainted.");
            return BattleResult::opponentfainted;
        case 3: 
            console.log("Battle ended"); // the opponent probably fled
            pbf_wait(context, 2000ms);
            context.wait_for_all_requests();
            return BattleResult::unknown;
        default:
            console.log("Failed to detect move use.");
            pbf_mash_button(context, BUTTON_B, 2000ms); // get back to the top-level battle menu
            context.wait_for_all_requests();
            continue;
        }
    }
}

void flee_battle(ConsoleHandle& console, ProControllerContext& context){
    uint16_t errors = 0;

    BattleMenuWatcher battle_menu(COLOR_RED);
    AdvanceBattleDialogWatcher ran_away(COLOR_YELLOW);
    BlackScreenOverWatcher battle_over(COLOR_RED);

    while (true)
    {
        if (errors > 5){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "flee_battle(): Failed to flee battle after 5 attempts.",
                console
            );
        }

        context.wait_for_all_requests();
        int ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context) {
                pbf_wait(context, 1000ms);
                pbf_mash_button(context, BUTTON_B, 9000ms);
            },
            { battle_menu }
        );
        if (ret < 0) {
            errors++;
            console.log("flee_battle(): Failed to detect battle menu. Attempt " + std::to_string(errors) + "/5", COLOR_RED);
            continue;
        }

        console.log("Navigate to Run.");
        pbf_press_dpad(context, DPAD_RIGHT, 160ms, 160ms);
        pbf_press_dpad(context, DPAD_DOWN, 160ms, 160ms);
        pbf_press_button(context, BUTTON_A, 160ms, 320ms);

        int ret2 = wait_until(
            console, context,
            std::chrono::seconds(5),
            { {ran_away} }
        );

        if (ret2 == 0) {
            console.log("Running away...");
        }
        else {
            // Even though we failed to detect the "ran away" dialog, we might have still successfully fled. 
            // Attempt the next detections as a possible recovery.
            console.log("flee_battle(): Failed to detect flee dialog.", COLOR_RED);
        }

        int ret3 = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context) {
                pbf_press_button(context, BUTTON_A, 320ms, 640ms);
                pbf_wait(context, 5000ms);
                context.wait_for_all_requests();
            },
            { battle_over, battle_menu }
        );
        if (ret3 == 0) {
            console.log("Successfully fled the battle.");
            return;
        }
        else if (ret3 == 1){
            errors++;
            console.log("flee_battle(): Detected battle menu after attempting to flee. Attempt " + std::to_string(errors) + "/5", COLOR_RED);
            continue;
        }
        else {
            console.log("flee_battle(): failed to detect transition to overworld. Attempting to open start menu to verify successful flee..", COLOR_RED);
            
            open_start_menu(console, context);
            close_start_menu(console, context);
            context.wait_for_all_requests();
            return;
        }
    }
}

bool exit_wild_battle(ConsoleHandle& console, ProControllerContext& context, bool stop_on_move_learn, bool prevent_evolution){
    // For move learning, there are two dialog selection boxes in a row
    // we need to decline the first one and accept the second one, so mashing B won't work
    // The first one will occur after an Advance Battle Dialog
    uint16_t errors = 0;
    uint16_t loops = 0;
    bool first_attempt = true;
    bool rejected_first_box = false;
    bool move_learned = false;
    while (true){
        if (errors > 5 || loops > 5){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "exit_wild_battle(): Failed to exit battle.",
                console
            );
        }

        BlackScreenWatcher battle_exited(COLOR_RED);    
        AdvanceBattleDialogWatcher advance_dialog(COLOR_RED);
        BattleLearnDialogWatcher move_learn_select(COLOR_RED);

        context.wait_for_all_requests();
        WallClock deadline = current_time() + 30s;
        int ret;
        if (first_attempt){
            ret = run_until<ProControllerContext>(
                console, context,
                [](ProControllerContext& context) {
                    pbf_mash_button(context, BUTTON_B, 20000ms);
                },
                { battle_exited }
            );
        }else{
            ret = run_until<ProControllerContext>(
                console, context,
                [deadline, rejected_first_box](ProControllerContext& context) {
                    pbf_wait(context, 1000ms); // give the watchers a chance to detect something
                    while (current_time() < deadline){
                        pbf_press_button(context, rejected_first_box ? BUTTON_A : BUTTON_B, 200ms, 1800ms);
                    }
                },
                { battle_exited, advance_dialog, move_learn_select }
            );
        }

        BattleDialogWatcher evolution_started(COLOR_RED);
        StartMenuWatcher start_menu_open(COLOR_RED);
        int ret2;

        switch (ret){
        case 0:
            // check for the evolution screen
            context.wait_for_all_requests(); 
            ret2 = run_until<ProControllerContext>(
                console, context,
                [](ProControllerContext& context) {
                    pbf_wait(context, 2000ms);
                },
                { evolution_started }
            );

            if (ret2 == 0){
                console.log("Evolution detected.");
                if (!prevent_evolution){
                    // make sure B isn't pressed too soon, which would cancel the evolution
                    pbf_wait(context, 20000ms);
                }
                rejected_first_box = false;
                continue; // press B as in other cases, and handle any move learning loops that might come up
            }
            console.log("Battle exited.");
            return move_learned;
        case 1:
            console.log("Battle Advance arrow detected.");
            pbf_press_button(context, BUTTON_B, 200ms, 800ms);
            rejected_first_box = false;
            continue;
        case 2:
            if (stop_on_move_learn){
                console.log("Move learn detected.");
                return true;
            }else if (rejected_first_box){
                loops++;
                console.log("Declined to learn new move.");
                pbf_press_button(context, BUTTON_A, 200ms, 0ms);
            }else{
                pbf_press_button(context, BUTTON_B, 200ms, 0ms);
                rejected_first_box = true;
                move_learned = true;
            }
            continue;
        default:
            if (first_attempt){
                console.log("Loop detected.");
                first_attempt = false;
                continue;
            }
            console.log("Failed to detect expected battle dialogs.");
            errors++;
            // attempt to exit any screen that might be open (party, bag, etc)
            pbf_mash_button(context, BUTTON_B, 500ms);
            // overworld detection: look for the start menu
            context.wait_for_all_requests();
            ret2 = run_until<ProControllerContext>(
                console, context,
                [](ProControllerContext& context) {
                    pbf_press_button(context, BUTTON_PLUS, 200ms, 800ms);
                    pbf_press_button(context, BUTTON_PLUS, 200ms, 800ms);
                    pbf_press_button(context, BUTTON_PLUS, 200ms, 800ms);
                },
                { start_menu_open }
            );
            if (ret2 == 0){
                pbf_mash_button(context, BUTTON_B, 500ms);
                context.wait_for_all_requests();
                console.log("Battle exited.");
                return move_learned;
            }
            context.wait_for_all_requests();
            rejected_first_box = false;
            continue;
        }
    }
}

void open_party_menu_from_overworld(ConsoleHandle& console, ProControllerContext& context, StartMenuContext menu_context){
    uint16_t errors = 0;
    bool start_menu_is_open = false;
    while (true){
        if (errors > 5){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "open_party_menu_from_overworld(): Failed to open party menu 5 times in a row.",
                console
            );
        }

        context.wait_for_all_requests();
        if (!start_menu_is_open){
            open_start_menu(console, context); // This is unavoidable since we cannot detect the overworld.
            start_menu_is_open = true;
        }

        StartMenuWatcher start_menu(COLOR_RED);
        PartyMenuWatcher party_menu(COLOR_RED);

        int ret = wait_until(
            console, context, 10000ms,
            { start_menu, party_menu }
        );

        switch (ret){
        case 0:
            if (menu_context == StartMenuContext::SAFARI_ZONE){
                ret = move_cursor_to_position(console, context, SelectionArrowPositionSafariMenu::POKEMON);
            } else {
                ret = move_cursor_to_position(console, context, SelectionArrowPositionStartMenu::POKEMON);
            }

            if (ret < 0){
                console.log("Failed to navigate to POKEMON on the start menu.");
                errors++;
                context.wait_for_all_requests();
                pbf_mash_button(context, BUTTON_B, 2000ms);
                start_menu_is_open = false;
            } else {
                console.log("Navigated to POKEMON on the start menu");
                context.wait_for_all_requests();
                pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
            }
            continue;
        case 1:
            console.log("Party menu opened.");
            return;
        default:
            console.log("Failed to open party menu.");
            errors++;
            pbf_mash_button(context, BUTTON_B, 2000ms);
            start_menu_is_open = false;
            continue;
        }
    }
}

void open_bag_from_overworld(ConsoleHandle& console, ProControllerContext& context, StartMenuContext menu_context){
    uint16_t errors = 0;
    bool start_menu_is_open = false;
    while (true){
        if (errors > 5){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "open_party_menu_from_overworld(): Failed to open party menu 5 times in a row.",
                console
            );
        }

        context.wait_for_all_requests();
        if (!start_menu_is_open){
            open_start_menu(console, context); // This is unavoidable since we cannot detect the overworld.
            start_menu_is_open = true;
        }

        StartMenuWatcher start_menu(COLOR_RED);
        BagWatcher bag(COLOR_RED);

        int ret = wait_until(
            console, context, 10000ms,
            { start_menu, bag }
        );

        switch (ret){
        case 0:
            if (menu_context == StartMenuContext::SAFARI_ZONE){
                ret = move_cursor_to_position(console, context, SelectionArrowPositionSafariMenu::BAG);
            } else {
                ret = move_cursor_to_position(console, context, SelectionArrowPositionStartMenu::BAG);
            }

            if (ret < 0){
                console.log("Failed to navigate to BAG on the start menu.");
                errors++;
                context.wait_for_all_requests();
                pbf_mash_button(context, BUTTON_B, 2000ms);
                start_menu_is_open = false;
            } else {
                console.log("Navigated to BAG on the start menu");
                context.wait_for_all_requests();
                pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
            }
            continue;
        case 1:
            console.log("Bag opened.");
            return;
        default:
            console.log("Failed to open bag.");
            errors++;
            pbf_mash_button(context, BUTTON_B, 2000ms);
            start_menu_is_open = false;
            continue;
        }
    }
}

void use_teleport_from_overworld(ConsoleHandle& console, ProControllerContext& context){
    uint16_t errors = 0;
    
    while (true){
        if (errors > 5){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "use_teleport_from_overworld(): Failed to use Teleport 5 times in a row.",
                console
            );
        }

        open_party_menu_from_overworld(console, context);
        // navigate to last party slot
        pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
        pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);

        PartySelectionWatcher teleporter_selected(COLOR_RED);

        context.wait_for_all_requests();
        int ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context){
                pbf_press_button(context, BUTTON_A, 200ms, 1800ms);
            },
            { teleporter_selected }
        );

        if (ret < 0){
            console.log("Failed to select Teleport user.");
            errors++;
            pbf_mash_button(context, BUTTON_B, 3000ms);
            continue;
        }
        
        // select Teleport (2nd option, but maybe HMs could change this)
        pbf_move_left_joystick(context, {0, -1}, 200ms, 300ms);
        pbf_press_button(context, BUTTON_A, 200ms, 1800ms);
        pbf_press_button(context, BUTTON_A, 200ms, 2800ms);

        BlackScreenWatcher teleport_transition(COLOR_RED);

        context.wait_for_all_requests();
        ret = wait_until(
            console, context, 20000ms,
            {teleport_transition}
        );

        if (ret < 0){
            console.log("Failed to use Teleport");
            errors++;
            pbf_mash_button(context, BUTTON_B, 4000ms);
            continue;
        }

        pbf_wait(context, 3000ms);
        context.wait_for_all_requests();
        console.log("Used Teleport.");
        return;
    }
}

void open_fly_map_from_overworld(ConsoleHandle& console, ProControllerContext& context){
    uint16_t errors = 0;
    while (true){
        if (errors > 5){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "open_fly_map_from_overworld(): Failed to open Fly map 5 times in a row.",
                console
            );
        }

        open_party_menu_from_overworld(console, context);
        // navigate to last party slot
        pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
        pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);

        PartySelectionWatcher fly_user_selected(COLOR_RED);

        context.wait_for_all_requests();
        int ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context) {
                pbf_press_button(context, BUTTON_A, 200ms, 1800ms);
            },
            { fly_user_selected }
        );

        if (ret < 0){
            console.log("Failed to select Fly user.");
            errors++;
            pbf_mash_button(context, BUTTON_B, 3000ms);
            continue;
        }
        
        // select Fly (2nd option, but maybe other HMs could change this)
        KantoMapWatcher map_opened(COLOR_RED);
        context.wait_for_all_requests();
        pbf_move_left_joystick(context, {0, -1}, 200ms, 300ms);
        pbf_press_button(context, BUTTON_A, 200ms, 0ms);
        ret = wait_until(
            console, context, 20000ms,
            {map_opened}
        );

        if (ret < 0){
            console.log("Failed to detect Kanto map");
            errors++;
            pbf_mash_button(context, BUTTON_B, 4000ms);
            continue;
        }

        pbf_wait(context, 1000ms);
        context.wait_for_all_requests();
        console.log("Kanto map detected.");
        return;
    }
}

void fly_from_kanto_map(ConsoleHandle& console, ProControllerContext& context, KantoFlyLocation destination){
    uint64_t errors = 0;
    
    while (true){
        if (errors > 5){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "fly_from_kanto_map(): Failed to inititate Fly five times in a row.",
                console
            ); 
        }

        pbf_move_left_joystick(context, {-1, +1}, 4000ms, 500ms);
        context.wait_for_all_requests();

        // blindly move the cursor to the specified fly spot
        switch (destination){
        case KantoFlyLocation::pallettown:
            pbf_move_left_joystick(context, {0, -1}, 850ms, 100ms);
            pbf_move_left_joystick(context, {+1, 0}, 317ms, 100ms);
            break;
        case KantoFlyLocation::viridiancity:
            pbf_move_left_joystick(context, {0, -1}, 633ms, 100ms);
            pbf_move_left_joystick(context, {+1, 0}, 317ms, 100ms);
            break;
        case KantoFlyLocation::pewtercity:
            pbf_move_left_joystick(context, {0, -1}, 317ms, 100ms);
            pbf_move_left_joystick(context, {+1, 0}, 317ms, 100ms);
            break;
        case KantoFlyLocation::route4:
            pbf_move_left_joystick(context, {0, -1}, 233ms, 100ms);
            pbf_move_left_joystick(context, {+1, 0}, 633ms, 100ms);
            break;
        case KantoFlyLocation::ceruleancity:
            pbf_move_left_joystick(context, {0, -1}, 233ms, 100ms);
            pbf_move_left_joystick(context, {+1, 0}, 1150ms, 100ms);
            break;
        case KantoFlyLocation::vermilioncity:
            pbf_move_left_joystick(context, {0, -1}, 700ms, 100ms);
            pbf_move_left_joystick(context, {+1, 0}, 1150ms, 100ms);
            break;
        case KantoFlyLocation::route10:
            pbf_move_left_joystick(context, {0, -1}, 233ms, 100ms);
            pbf_move_left_joystick(context, {+1, 0}, 1500ms, 100ms);
            break;
        case KantoFlyLocation::lavendertown:
            pbf_move_left_joystick(context, {0, -1}, 467ms, 100ms);
            pbf_move_left_joystick(context, {+1, 0}, 1500ms, 100ms);
            break;
        case KantoFlyLocation::celadoncity:
            pbf_move_left_joystick(context, {0, -1}, 467ms, 100ms);
            pbf_move_left_joystick(context, {+1, 0}, 900ms, 100ms);
            break;
        case KantoFlyLocation::saffroncity:
            pbf_move_left_joystick(context, {0, -1}, 467ms, 100ms);
            pbf_move_left_joystick(context, {+1, 0}, 1150ms, 100ms);
            break;
        case KantoFlyLocation::fuschiacity:
            pbf_move_left_joystick(context, {0, -1}, 967ms, 100ms);
            pbf_move_left_joystick(context, {+1, 0}, 967ms, 100ms);
            break;
        case KantoFlyLocation::cinnabarisland:
            pbf_move_left_joystick(context, {0, -1}, 1100ms, 100ms);
            pbf_move_left_joystick(context, {+1, 0}, 317ms, 100ms);
            break;
        case KantoFlyLocation::indigoplateau:
            pbf_move_left_joystick(context, {0, -1}, 200ms, 100ms);
            pbf_move_left_joystick(context, {+1, 0}, 150ms, 100ms);
            break;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "fly_from_kanto_map(): Unimplemented Kanto fly target.",
                console
            );
        }

        BlackScreenWatcher fly_initiated(COLOR_RED);
        context.wait_for_all_requests();
        int ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context) {
                // walk up to counter and initiate dialog
                pbf_mash_button(context, BUTTON_A, 5000ms);
            },
            { fly_initiated }
        );
        
        pbf_wait(context, 8000ms);
        context.wait_for_all_requests();

        if (ret == 0) {
            console.log("Fly initiated.");
            return;
        }else{
            errors++;
            console.log("Failed to detect black screen within 5 seconds of attempting to fly.");
            continue;
        }

    }
    
}

void enter_leave_pokecenter(ConsoleHandle& console, ProControllerContext& context, bool leave){
    uint16_t errors = 0;

    while (true){
        if (errors > 5){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                leave ? "leave_pokecenter(): Failed to exit PokeCenter." : "enter_pokecenter(): Failed to enter PokeCenter.",
                console
            );
        }

        BlackScreenWatcher pokecenter_transition(COLOR_RED);

        int ret = run_until<ProControllerContext>(
            console, context,
            [leave](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, (leave ? -1.0 : +1.0)}, 10000ms, 0ms);
            },
            { pokecenter_transition }
        );

        if (ret < 0){
            console.log(leave ? "Failed to exit PokeCenter." : "Failed to enter PokeCenter.");
            errors++;
            pbf_mash_button(context, BUTTON_B, 1000ms);
            continue;
        }

        pbf_wait(context, 2500ms);
        context.wait_for_all_requests();
        console.log(leave ? "Exited PokeCenter." : "Entered PokeCenter");
        return;
    }
}

void enter_pokecenter(ConsoleHandle& console, ProControllerContext& context){
    enter_leave_pokecenter(console, context, false);
}

void leave_pokecenter(ConsoleHandle& console, ProControllerContext& context){
    enter_leave_pokecenter(console, context, true);
}

void heal_at_pokecenter(ConsoleHandle& console, ProControllerContext& context){
    uint16_t errors = 0;

    while (true){
        if (errors > 5){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "heal_at_pokecenter(): Failed to initiate PokeCenter dialog.",
                console
            );
        }

        AdvanceWhiteDialogWatcher dialog(COLOR_RED);

        context.wait_for_all_requests();
        int ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context){
                // walk up to counter and initiate dialog
                ssf_press_left_joystick(context, {0, +1}, 0ms, 10000ms);
                ssf_mash1_button(context, BUTTON_A, 10000ms);
            },
            { dialog }
        );

        if (ret < 0){
            console.log("Failed to detect PokeCenter dialog within 10 seconds");
            errors++;
            pbf_mash_button(context, BUTTON_B, 2000ms);
            continue;
        }

        console.log("Detected PokeCenter dialog.");
        pbf_mash_button(context, BUTTON_A, 8000ms);
        pbf_mash_button(context, BUTTON_B, 5000ms);
        context.wait_for_all_requests();
        return;
    }
}

int grass_spin(ConsoleHandle& console, ProControllerContext& context, bool leftright, Seconds timeout){
    BlackScreenWatcher battle_triggered(COLOR_RED);
    BattleDialogWatcher battle_entered(COLOR_RED);

    context.wait_for_all_requests();
    console.log("Starting grass spin.");
    WallClock deadline = current_time() + timeout;

    int ret = run_until<ProControllerContext>(
        console, context,
        [leftright, deadline](ProControllerContext& context){
            while (current_time() < deadline){
                if (leftright){
                    pbf_move_left_joystick(context, {+1, 0}, 33ms, 150ms);
                    pbf_move_left_joystick(context, {-1, 0}, 33ms, 150ms);
                }else{
                    pbf_move_left_joystick(context, {0, +1}, 33ms, 150ms);
                    pbf_move_left_joystick(context, {0, -1}, 33ms, 150ms);
                }
            }
        },
        { battle_triggered, battle_entered }
    );

    if (ret < 0){
        return -1;
    }

    bool encounter_shiny = handle_encounter(console, context, true);
    return encounter_shiny ? 1 : 0;
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
        console.log("Skipping black border check.", COLOR_BLUE);
    }
}


}
}
}
