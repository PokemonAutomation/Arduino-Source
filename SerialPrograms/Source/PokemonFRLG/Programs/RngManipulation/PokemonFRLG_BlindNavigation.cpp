/*  Blind Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Random.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonFRLG_BlindNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


void set_seed_after_delay(ProControllerContext& context, SeedButton SEED_BUTTON, BlackoutButton BLACKOUT_BUTTON, int64_t SEED_DELAY){
    // wait on title screen for the specified delay    
    // hold the "blackout" button starting from the black screen after the copyright text until getting to the continue screen
    if (BLACKOUT_BUTTON != BlackoutButton::None){
        Button b_button;
        switch (BLACKOUT_BUTTON){
        case BlackoutButton::L:
            b_button = BUTTON_L;
            break;
        case BlackoutButton::R:
            b_button = BUTTON_R;
            break;
        default:
            b_button = BUTTON_L;
        }
        Milliseconds blackout_wait = 3600ms; // wait for the copyright text to disappear
        Milliseconds blackout_delay = std::chrono::milliseconds(SEED_DELAY) - blackout_wait;
        Milliseconds blackout_hold = 30000ms; // wait for leaves/flames to appear on the title screen. It's okay if this is held over the seed button press
        ssf_do_nothing(context, blackout_wait);
        ssf_press_button(context, b_button, blackout_delay, blackout_hold, 0ms);
    }else{
        pbf_wait(context, std::chrono::milliseconds(SEED_DELAY));
    }

    // hold the specified button for a few seconds through the transition to the Continue Screen
    Button s_button;
    switch (SEED_BUTTON){
    case SeedButton::A:
        s_button = BUTTON_A;
        break;
    case SeedButton::Start:
        s_button = BUTTON_PLUS;
        break;
    case SeedButton::L:
        s_button = BUTTON_L;
        break;
    default:
        s_button = BUTTON_A;
        break;
    }
    pbf_press_button(context, s_button, 3000ms, 0ms);
}

void load_game_after_delay(ProControllerContext& context, uint64_t CONTINUE_SCREEN_DELAY){
    pbf_wait(context, std::chrono::milliseconds(CONTINUE_SCREEN_DELAY - 3000));
    pbf_press_button(context, BUTTON_A, 50ms, 1450ms);
    // skip recap
    pbf_press_button(context, BUTTON_B, 50ms, 2450ms);
    // need to later subtract 4000ms from delay to hit desired number of advances
}

void wait_with_teachy_tv(ProControllerContext& context, uint64_t TEACHY_DELAY){
    // open start menu -> bag -> key items -> Teachy TV -> use
    pbf_press_button(context, BUTTON_PLUS, 200ms, 300ms);
    pbf_move_left_joystick(context, {0, -1}, 200ms, 300ms);
    pbf_move_left_joystick(context, {0, -1}, 200ms, 300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 2300ms);
    pbf_move_left_joystick(context, {+1, 0}, 200ms, 2300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 300ms);
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(TEACHY_DELAY));
    // close teachy tv -> close bag -> reset start menu cursor position - > close start menu
    pbf_press_button(context, BUTTON_B, 200ms, 2300ms);
    pbf_press_button(context, BUTTON_B, 200ms, 2300ms);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    pbf_press_button(context, BUTTON_B, 200ms, 300ms);
    // total non-teachy delay duration: 13700ms
}


void collect_starter_after_delay(ProControllerContext& context, uint64_t INGAME_DELAY){
    // Advance through starter dialogue and wait on "really quite energetic!"
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(INGAME_DELAY - 7200)); // 4000ms + 3000ms + 200ms
    // Finish dialogue (hits the target advance)
    pbf_press_button(context, BUTTON_A, 200ms, 5800ms);
    // Decline nickname
    pbf_mash_button(context, BUTTON_B, 2500ms);
    // Advance through rival choice
    pbf_mash_button(context, BUTTON_B, 5000ms);
    context.wait_for_all_requests();
}

void collect_magikarp_after_delay(ProControllerContext& context, uint64_t INGAME_DELAY){
    // Advance through starter dialogue and wait on YES/NO
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(INGAME_DELAY - 7200)); // 4000ms + 3000ms + 200ms
    // Finish dialogue (hits the target advance)
    pbf_press_button(context, BUTTON_A, 200ms, 3800ms);
    // Decline nickname
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
}

void collect_hitmon_after_delay(ProControllerContext& context, uint64_t INGAME_DELAY){
    // One dialog before accepting
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(INGAME_DELAY - 4200)); // 4000ms + 200ms
    // Confirm selection
    pbf_press_button(context, BUTTON_A, 200ms, 1800ms);
    // Decline nickname
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
}

void collect_eevee_after_delay(ProControllerContext& context, uint64_t INGAME_DELAY){
    // No dialogue to advance through -- just wait
    pbf_wait(context, std::chrono::milliseconds(INGAME_DELAY - 4000));
    // Interact with the pokeball
    pbf_press_button(context, BUTTON_A, 200ms, 3800ms);
    // Decline nickname
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
}

void collect_lapras_after_delay(ProControllerContext& context, uint64_t INGAME_DELAY){
    // 3 dialog presses
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(INGAME_DELAY - 7200)); // 4000ms + 3000ms + 200ms
    // Accept Lapras on target frame
    pbf_press_button(context, BUTTON_A, 200ms, 3800ms);
    // Decline nickname and exit dialog
    pbf_mash_button(context, BUTTON_B, 7500ms);
    context.wait_for_all_requests();
}

void collect_fossil_after_delay(ProControllerContext& context, uint64_t INGAME_DELAY){
    // 2 dialog presses
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(INGAME_DELAY - 5700)); // 4000ms + 1500ms + 200ms
    // Advance dialog on target frame
    pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
    // Decline nickname
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
}

void collect_gamecorner_after_delay(ProControllerContext& context, uint64_t INGAME_DELAY, int SLOT){
    // 2 dialog presses
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    // navigate to desired option
    for (int i=0; i<SLOT; i++){
        pbf_move_left_joystick(context, {0,-1}, 100ms, 300ms);
    }
    for (int i=SLOT; i<5; i++){
        pbf_wait(context, 400ms);
    }
    // select option and wait on confirmation
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(INGAME_DELAY - 8200)); // 4000ms + 3000ms + (400ms * 5) + 200ms
    // confirm prize
    pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
    // decline nickname
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
}

void collect_togepi_egg_after_delay(ProControllerContext& context, uint64_t INGAME_DELAY){
    // 6 dialog presses
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(INGAME_DELAY - 11700)); // 4000ms + 7500ms + 200ms
    // accept egg
    pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
    // exit dialogue
    pbf_mash_button(context, BUTTON_B, 2500ms);
    context.wait_for_all_requests();    
}

void encounter_static_after_delay(ProControllerContext& context, uint64_t INGAME_DELAY){
    // No dialogue to advance through -- just wait in the start menu (avoids extra RNG advances by boulders Mt Ember and Seafoam Islands)
    pbf_press_button(context, BUTTON_PLUS, 200ms, 300ms);
    pbf_wait(context, std::chrono::milliseconds(INGAME_DELAY - 5000)); // 4000ms + 1000ms
    pbf_press_button(context, BUTTON_B, 200ms, 300ms);
    // Interact with the static encounter
    pbf_press_button(context, BUTTON_A, 200ms, 800ms);
    pbf_mash_button(context, BUTTON_A, 1000ms); // finishes dialog for the legendary birds
    context.wait_for_all_requests();
}

void encounter_snorlax_after_delay(ProControllerContext& context, uint64_t INGAME_DELAY){
    // Interact with Snorlax, YES to PokeFlute, wait on "woke up!"
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 9800ms); // PokeFlute tune
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(INGAME_DELAY - 15700)); // 4000ms + 1500ms + 10000ms + 200ms
    pbf_press_button(context, BUTTON_A, 200ms, 200ms); 
    context.wait_for_all_requests();
}

void encounter_mewtwo_after_delay(ProControllerContext& context, uint64_t INGAME_DELAY){
    // one dialogue before the encounter happens
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(INGAME_DELAY - 4200)); // 4000ms + 200ms
    // Initiate encounter
    pbf_press_button(context, BUTTON_A, 200ms, 200ms); 
    context.wait_for_all_requests();
}

void encounter_hooh_after_delay(ProControllerContext& context, uint64_t INGAME_DELAY){
    // No dialogue to advance through -- just wait
    pbf_wait(context, std::chrono::milliseconds(INGAME_DELAY - 4000));
    // Trigger the encounter (WALK UP)
    pbf_move_left_joystick(context, {0, +1}, 800ms, 700ms);
    context.wait_for_all_requests();
}

void encounter_hypno_after_delay(ProControllerContext& context, uint64_t INGAME_DELAY){
    // 5 dialog advances, with the 5th needing some extra time
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 2300ms);
    // Wait after the 6th
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(INGAME_DELAY - 12700)); // 4000ms + 8500ms + 200ms
    // Initiate encounter
    pbf_press_button(context, BUTTON_A, 200ms, 200ms); 
    context.wait_for_all_requests();
}

void use_sweet_scent(ProControllerContext& context, uint64_t INGAME_DELAY, bool SAFARI_ZONE = false){
    // navigate to last party slot
    pbf_press_button(context, BUTTON_PLUS, 200ms, 300ms);
    pbf_move_left_joystick(context, {0, -1}, 200ms, 300ms);
    if (SAFARI_ZONE) { // there is an extra menu option at the top of the start menu
        pbf_move_left_joystick(context, {0, -1}, 200ms, 300ms); 
    }
    pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 300ms);
    // hover over Sweet Scent (2nd option, but maybe HMs could change this)
    pbf_move_left_joystick(context, {0, -1}, 200ms, std::chrono::milliseconds(INGAME_DELAY - (SAFARI_ZONE ? 7900 : 7400)));
    pbf_press_button(context, BUTTON_A, 200ms, 800ms);
    context.wait_for_all_requests();
}

void use_registered_fishing_rod(ProControllerContext& context, uint64_t INGAME_DELAY){
    uint32_t rng_wait = 50 * random_u32(0, 20); // helps avoid always hitting "Not even a nibble" (?)
    pbf_wait(context, std::chrono::milliseconds(rng_wait));
    pbf_press_button(context, BUTTON_MINUS, 200ms, std::chrono::milliseconds(INGAME_DELAY - rng_wait - 4200));
    pbf_press_button(context, BUTTON_A, 200ms, 800ms);
    context.wait_for_all_requests();
}

void enter_safarizone(ProControllerContext& context){
    // walk up to initiate dialogue
    pbf_move_left_joystick(context, {0, +1}, 600ms, 400ms);
    // Advance through the dialogue (waiting a little longer when Safari Balls are recieved)
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 3300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    // finish dialogue and automatically enter the Safari Zone
    pbf_press_button(context, BUTTON_A, 200ms, 4800ms);
    // total duration: 18500ms
}

void walk_to_safarizonefish(ProControllerContext& context){
    enter_safarizone(context); // 18500ms
    // walk from the entrance to the pond in the central area
    pbf_move_left_joystick(context, {0, +1}, 2200ms, 300ms);
    pbf_move_left_joystick(context, {+1, 0}, 1600ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 600ms, 300ms);
    // total duration: 23800ms
}

void walk_to_safarizonesurf(ProControllerContext& context){
    walk_to_safarizonefish(context); // 23800ms
    // start surfing
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 3300ms);
    // total duration: 30300ms
}

void walk_to_safarizonecenter(ProControllerContext& context){
    enter_safarizone(context); // 18500ms
    // walk from the entrance to the nearest grass
    pbf_move_left_joystick(context, {0, +1}, 460ms, 300ms);
    pbf_move_left_joystick(context, {-1, 0}, 1110ms, 300ms);
    // total duration: 20670ms
}

void walk_to_safarizoneeast(ProControllerContext& context){
    enter_safarizone(context); // 18500ms
    // walk from the entrance to the east area
    pbf_move_left_joystick(context, {0, +1}, 160ms, 300ms);
    pbf_move_left_joystick(context, {+1, 0}, 4400ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 3550ms, 300ms);
    // walk to the nearest grass
    pbf_move_left_joystick(context, {+1, 0}, 3600ms, 300ms);
    pbf_move_left_joystick(context, {0, -1}, 700ms, 300ms);
    pbf_move_left_joystick(context, {+1, 0}, 3450ms, 300ms);
    // total duration: 36160ms
}

void walk_to_safarizonenorth(ProControllerContext& context){
    walk_to_safarizonesurf(context); // 30300ms
    // from the pond to the grass in the north area
    pbf_move_left_joystick(context, {0, +1}, 2810ms, 300ms);
    pbf_move_left_joystick(context, {-1, 0}, 1530ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 1870ms, 300ms);
    // total duration: 37410ms
}

void walk_to_safarizonewest(ProControllerContext& context){
    walk_to_safarizonesurf(context); // 30300ms
    // surf past the hedge and exit the pond
    pbf_move_left_joystick(context, {-1, 0}, 500ms, 300ms);
    pbf_move_left_joystick(context, {0, -1}, 260ms, 500ms);
    // walk to the west area
    pbf_move_left_joystick(context, {-1, 0}, 5500ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 240ms, 300ms);
    pbf_move_left_joystick(context, {-1, 0}, 1200ms, 500ms);
    // walk to the grass
    pbf_move_left_joystick(context, {-1, 0}, 2860ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 1390ms, 300ms);
    pbf_move_left_joystick(context, {-1, 0}, 1510ms, 300ms);
    pbf_move_left_joystick(context, {0, -1}, 770ms, 300ms);
    pbf_move_left_joystick(context, {-1, 0}, 2400ms, 300ms);
    pbf_move_left_joystick(context, {0, -1}, 600ms, 300ms);
    // total duration: 51430ms
}


void check_timings(
    ConsoleHandle& console, 
    PokemonFRLG_RngTarget TARGET,
    uint64_t SEED_DELAY,
    uint64_t CONTINUE_SCREEN_DELAY, 
    uint64_t INGAME_DELAY,
    bool SAFARI_ZONE
){
    if (CONTINUE_SCREEN_DELAY < 3200){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "The Continue Screen delay cannot be less than 3200ms (192 advances). Check your Continue Screen calibration.",
            console
        );
    }
    if (SEED_DELAY < 28000){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "The title screen delay cannot be less than 28000ms. Check your seed calibration.",
            console
        );
    }
    
    switch (TARGET){
    case PokemonFRLG_RngTarget::starters:
        if (INGAME_DELAY < 7500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Starters: the in-game delay cannot be less than 7500ms (900 advances). Check your in-game advances and calibration.",
                console
            );
        }
        return;
    case PokemonFRLG_RngTarget::magikarp:
        if (INGAME_DELAY < 7500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Magikarp: the in-game delay cannot be less than 7500ms (900 advances). Check your in-game advances and calibration.",
                console
            );
        }
        return;
    case PokemonFRLG_RngTarget::hitmonchan:
    case PokemonFRLG_RngTarget::hitmonlee:
    case PokemonFRLG_RngTarget::hitmon:
        if (INGAME_DELAY < 4500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Hitmonchan/Hitmonlee: the in-game delay cannot be less than 4500ms (540 advances). Check your in-game advances and calibration.",
                console
            );
        }
        return;
    case PokemonFRLG_RngTarget::eevee:
        if (INGAME_DELAY < 4000){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Eevee: the in-game delay cannot be less than 4000ms (480 advances). Check your in-game advances and calibration.",
                console
            );
        }
        return;
    case PokemonFRLG_RngTarget::lapras:
        if (INGAME_DELAY < 7500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Lapras: the in-game delay cannot be less than 7500ms (900 advances). Check your in-game advances and calibration.",
                console
            );
        }
        return;
    case PokemonFRLG_RngTarget::omanyte:
    case PokemonFRLG_RngTarget::kabuto:
    case PokemonFRLG_RngTarget::aerodactyl:
    case PokemonFRLG_RngTarget::fossils:
        if (INGAME_DELAY < 6000){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Fossils: the in-game delay cannot be less than 6000ms (720 advances). Check your in-game advances and calibration.",
                console
            );
        }
        return;
    case PokemonFRLG_RngTarget::gamecornerabra:
    case PokemonFRLG_RngTarget::gamecornerclefairy:
    case PokemonFRLG_RngTarget::gamecornerdratini:
    case PokemonFRLG_RngTarget::gamecornerscyther:
    case PokemonFRLG_RngTarget::gamecornerpinsir:
    case PokemonFRLG_RngTarget::gamecornerbug:
    case PokemonFRLG_RngTarget::gamecornerporygon:
        if (INGAME_DELAY < 8500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Game Corner: the in-game delay cannot be less than 8500ms (1020 advances). Check your in-game advances and calibration.",
                console
            );
        }
        return;
    case PokemonFRLG_RngTarget::togepi:
        if (INGAME_DELAY < 12000) {
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Togepi: the in-game delay cannot be less than 12000ms (1440 advances). Check your in-game advances and calibration.",
                console
            );
        }
        return;
    case PokemonFRLG_RngTarget::staticencounter:
        if (INGAME_DELAY < 5000){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Static Encounter: the in-game delay cannot be less than 5000ms (600 advances). Check your in-game advances and calibration.",
                console
            );
        }
        return;
    case PokemonFRLG_RngTarget::snorlax:
        if (INGAME_DELAY < 16000){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Snorlax: the in-game delay cannot be less than 16000ms (1920 advances). Check your in-game advances and calibration.",
                console
            );
        }
        return;
    case PokemonFRLG_RngTarget::mewtwo:
        if (INGAME_DELAY < 4500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Mewtwo: the in-game delay cannot be less than 4500ms (540 advances). Check your in-game advances and calibration.",
                console
            );
        }
        return;
    case PokemonFRLG_RngTarget::hooh:
        if (INGAME_DELAY < 4000){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Ho-oh: the in-game delay cannot be less than 4000ms (480 advances). Check your in-game advances and calibration.",
                console
            );
        }
        return;
    case PokemonFRLG_RngTarget::hypno:
        if (INGAME_DELAY < 13000){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Hypno: the in-game delay cannot be less than 13000ms (1560 advances). Check your in-game advances and calibration.",
                console
            );
        }
        return;
    case PokemonFRLG_RngTarget::sweetscent:
        if (!SAFARI_ZONE && INGAME_DELAY < 8500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Sweet Scent: the in-game delay cannot be less than 8500ms (1020 advances). Check your in-game advances and calibration.",
                console
            );
        }else if (SAFARI_ZONE && INGAME_DELAY < 9500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Sweet Scent: the in-game delay cannot be less than 9500ms (1140 advances). Check your in-game advances and calibration.",
                console
            );
        }
        return;
    case PokemonFRLG_RngTarget::fishing:
        if (INGAME_DELAY < 5500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Fishing: the in-game delay cannot be less than 5500ms (1800 advances). Check your in-game advances and calibration.",
                console
            );
        }
        return;
    case PokemonFRLG_RngTarget::safarizonecenter:
        if (INGAME_DELAY < 30500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Safari Zone Center: in-game delay cannot be less than 30500ms (3660 advances). Check your in-game advances and calibration.",
                console
            );
        }
        return;
    case PokemonFRLG_RngTarget::safarizoneeast:
        if (INGAME_DELAY < 36500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Safari Zone East: in-game delay cannot be less than 36500ms (4380 advances). Check your in-game advances and calibration.",
                console
            );
        }
        return;
    case PokemonFRLG_RngTarget::safarizonenorth:
        if (INGAME_DELAY < 47500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Safari Zone North: in-game delay cannot be less than 47500ms (5700 advances). Check your in-game advances and calibration.",
                console
            );
        }
        return;
    case PokemonFRLG_RngTarget::safarizonewest:
        if (INGAME_DELAY < 61500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Safari Zone West: in-game delay cannot be less than 52000ms (7380 advances). Check your in-game advances and calibration.",
                console
            );
        }
        return;
    case PokemonFRLG_RngTarget::safarizonesurf:
        if (INGAME_DELAY < 40500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Safari Zone Surfing: in-game delay cannot be less than 40500ms (4860 advances). Check your in-game advances and calibration.",
                console
            );
        }
        return;
    case PokemonFRLG_RngTarget::safarizonefish:
        if (INGAME_DELAY < 30000){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Safari Zone Fishing: in-game delay cannot be less than 30000ms (3600 advances). Check your in-game advances and calibration.",
                console
            );
        }
        return;
    default:
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Option not yet implemented.",
            console
        );
    }
}

void perform_blind_sequence(
    ProControllerContext& context, 
    PokemonFRLG_RngTarget TARGET,
    SeedButton SEED_BUTTON,
    BlackoutButton BLACKOUT_BUTTON,
    uint64_t SEED_DELAY,
    uint64_t CONTINUE_SCREEN_DELAY, 
    uint64_t TEACHY_DELAY, 
    uint64_t INGAME_DELAY, 
    bool SAFARI_ZONE
){
    pbf_press_button(context, BUTTON_A, 80ms, 0ms); // start the game from the Home screen
    set_seed_after_delay(context, SEED_BUTTON, BLACKOUT_BUTTON, SEED_DELAY);
    load_game_after_delay(context, CONTINUE_SCREEN_DELAY);
    if (TEACHY_DELAY > 0){
        wait_with_teachy_tv(context, TEACHY_DELAY);
    }

    uint64_t MODIFIED_INGAME_DELAY;
    switch (TARGET){
    case PokemonFRLG_RngTarget::starters:
        collect_starter_after_delay(context, INGAME_DELAY);
        return;
    case PokemonFRLG_RngTarget::magikarp:
        collect_magikarp_after_delay(context, INGAME_DELAY);
        return;
    case PokemonFRLG_RngTarget::hitmonchan:
    case PokemonFRLG_RngTarget::hitmonlee:
    case PokemonFRLG_RngTarget::hitmon:
        collect_hitmon_after_delay(context, INGAME_DELAY);
        return;
    case PokemonFRLG_RngTarget::eevee:
        collect_eevee_after_delay(context, INGAME_DELAY);
        return;
    case PokemonFRLG_RngTarget::lapras:
        collect_lapras_after_delay(context, INGAME_DELAY);
        return;
    case PokemonFRLG_RngTarget::omanyte:
    case PokemonFRLG_RngTarget::kabuto:
    case PokemonFRLG_RngTarget::aerodactyl:
    case PokemonFRLG_RngTarget::fossils:
        collect_fossil_after_delay(context, INGAME_DELAY);
        return;
    case PokemonFRLG_RngTarget::gamecornerabra:
        collect_gamecorner_after_delay(context, INGAME_DELAY, 0);
        return;
    case PokemonFRLG_RngTarget::gamecornerclefairy:
        collect_gamecorner_after_delay(context, INGAME_DELAY, 1);
        return;
    case PokemonFRLG_RngTarget::gamecornerdratini:
        collect_gamecorner_after_delay(context, INGAME_DELAY, 2);
        return;
    case PokemonFRLG_RngTarget::gamecornerscyther:
    case PokemonFRLG_RngTarget::gamecornerpinsir:
    case PokemonFRLG_RngTarget::gamecornerbug:
        collect_gamecorner_after_delay(context, INGAME_DELAY, 3);
        return;
    case PokemonFRLG_RngTarget::gamecornerporygon:
        collect_gamecorner_after_delay(context, INGAME_DELAY, 4);
        return;
    case PokemonFRLG_RngTarget::togepi:
        collect_togepi_egg_after_delay(context, INGAME_DELAY);
        return;
    case PokemonFRLG_RngTarget::staticencounter:
        encounter_static_after_delay(context, INGAME_DELAY);
        return;
    case PokemonFRLG_RngTarget::snorlax:
        encounter_snorlax_after_delay(context, INGAME_DELAY);
        return;
    case PokemonFRLG_RngTarget::mewtwo:
        encounter_mewtwo_after_delay(context, INGAME_DELAY);
        return;
    case PokemonFRLG_RngTarget::hooh:
        encounter_hooh_after_delay(context, INGAME_DELAY);
        return;
    case PokemonFRLG_RngTarget::hypno:
        encounter_hypno_after_delay(context, INGAME_DELAY);
        return;
    case PokemonFRLG_RngTarget::sweetscent:
        use_sweet_scent(context, INGAME_DELAY, SAFARI_ZONE);
        return;
    case PokemonFRLG_RngTarget::fishing:
        use_registered_fishing_rod(context, INGAME_DELAY);
        return;
    case PokemonFRLG_RngTarget::safarizonecenter:
        MODIFIED_INGAME_DELAY = INGAME_DELAY - 20670;
        walk_to_safarizonecenter(context);
        use_sweet_scent(context, MODIFIED_INGAME_DELAY, true);
        return;
    case PokemonFRLG_RngTarget::safarizoneeast:
        MODIFIED_INGAME_DELAY = INGAME_DELAY - 36160;
        walk_to_safarizoneeast(context);
        use_sweet_scent(context, MODIFIED_INGAME_DELAY, true);
        return;
    case PokemonFRLG_RngTarget::safarizonenorth:
        MODIFIED_INGAME_DELAY = INGAME_DELAY - 37410;
        walk_to_safarizonenorth(context);
        use_sweet_scent(context, MODIFIED_INGAME_DELAY, true);
        return;
    case PokemonFRLG_RngTarget::safarizonewest:
        MODIFIED_INGAME_DELAY = INGAME_DELAY - 51430;
        walk_to_safarizonewest(context);
        use_sweet_scent(context, MODIFIED_INGAME_DELAY, true);
    case PokemonFRLG_RngTarget::safarizonesurf:
        MODIFIED_INGAME_DELAY = INGAME_DELAY - 30300;
        walk_to_safarizonesurf(context);
        use_sweet_scent(context, MODIFIED_INGAME_DELAY, true);
        return;
    case PokemonFRLG_RngTarget::safarizonefish:
        MODIFIED_INGAME_DELAY = INGAME_DELAY - 30300;
        walk_to_safarizonefish(context);
        use_registered_fishing_rod(context, MODIFIED_INGAME_DELAY);
        return;
    }
}

}
}
}