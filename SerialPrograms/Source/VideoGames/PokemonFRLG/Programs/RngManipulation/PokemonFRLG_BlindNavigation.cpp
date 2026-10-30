/*  Blind Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <string>
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonTools/Random.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonFRLG_BlindNavigation.h"
#include "PokemonFRLG_RngCalibration.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

// The fixed parts (ms) of the button press sequences for each target,
// including the 4000ms spent loading the save and skipping the recap in load_game_after_delay().
static const uint64_t STARTER_FIXED_DURATION        = 7200;     // 4000ms + 3000ms + 200ms
static const uint64_t HITMON_FIXED_DURATION         = 4200;     // 4000ms + 200ms
static const uint64_t EEVEE_FIXED_DURATION          = 4000;
static const uint64_t FOSSIL_FIXED_DURATION         = 5700;     // 4000ms + 1500ms + 200ms
static const uint64_t GAMECORNER_FIXED_DURATION     = 8200;     // 4000ms + 3000ms + (400ms * 5) + 200ms
static const uint64_t TOGEPI_FIXED_DURATION         = 11700;    // 4000ms + 7500ms + 200ms
static const uint64_t TOGEPI_FAST_FIXED_DURATION    = 5200;     // 4000ms + 500ms + 500ms + 200ms
static const uint64_t EGG_HELD_FIXED_DURATION       = 4000;
static const uint64_t EGG_PICKUP_FIXED_DURATION     = 12000;    // 4000ms + 500ms + 500ms + 6000ms + 1000ms
static const uint64_t STATIC_FIXED_DURATION         = 5000;     // 4000ms + 1000ms
static const uint64_t SNORLAX_FIXED_DURATION        = 15700;    // 4000ms + 1500ms + 10000ms + 200ms
static const uint64_t MEWTWO_FIXED_DURATION         = 4200;     // 4000ms + 200ms
static const uint64_t HOOH_FIXED_DURATION           = 4000;
static const uint64_t HYPNO_FIXED_DURATION          = 12700;    // 4000ms + 8500ms + 200ms
static const uint64_t SWEET_SCENT_FIXED_DURATION    = 7400;
static const uint64_t SAFARI_SWEET_SCENT_FIXED_DURATION = 7900; // there is an extra menu option at the top of the start menu
static const uint64_t ROCK_SMASH_FIXED_DURATION     = 6500;     // 4000ms + 2000ms + 500ms
static const uint64_t FISHING_FIXED_DURATION        = 4200;     // 4000ms + 200ms
static const uint64_t FISHING_MAX_RNG_WAIT          = 1000;     // 50ms * 20. The random wait before casting comes out of the variable wait

// The part of the in-game delay (ms) that is spent walking to each Safari Zone target.
static const uint64_t SAFARI_CENTER_WALK_DURATION   = 22170;
static const uint64_t SAFARI_EAST_WALK_DURATION     = 37660;
static const uint64_t SAFARI_NORTH_WALK_DURATION    = 38910;
static const uint64_t SAFARI_WEST_WALK_DURATION     = 52930;
static const uint64_t SAFARI_SURF_WALK_DURATION     = 31800;
static const uint64_t SAFARI_FISH_WALK_DURATION     = 25300;

// The minimum delay (ms) before the final button press to avoid button press drops
static const uint64_t MIN_PRESS_GAP = 300;

// frames, approximate
static const int64_t DEFAULT_ADVANCES_OFFSET = 160;
static const int64_t SWEET_SCENT_ADVANCES_OFFSET = -352;

static uint64_t magikarp_fixed_duration(Language language){
    uint64_t duration = 7200; // 4000ms + 3000ms + 200ms
    if (language == Language::French){
        duration -= 1500;
    }
    if (language == Language::Japanese){
        duration += 1500;
    }
    return duration;
}

static uint64_t lapras_fixed_duration(Language language){
    uint64_t duration = 7200; // 4000ms + 3000ms + 200ms
    if (language == Language::French){
        duration -= 1500;
    }
    return duration;
}

static uint64_t roamer_fixed_duration(Language language){
    uint64_t duration = 26700; // 5500ms + 4*1500ms + 3500ms + 5500ms + 4*1500 + 200
    if (language == Language::Japanese){
        duration += 1500; 
    }else if (language == Language::Spanish){
        duration += 3000;
    }
    return duration;
}


void set_seed_after_delay(ProControllerContext& context, SeedButton seed_button, BlackoutButton extra_button, int64_t seed_delay, ConsoleType console_type){
    // be warned: not tested with all console types
    switch (console_type){
    case ConsoleType::Switch1:
        // Switch 1 enters the game a little bit earlier
        pbf_wait(context, 750ms);
        break;
    default:
        break;
    }

    // wait on title screen for the specified delay    
    // hold the "blackout" button starting from the black screen after the copyright text until getting to the continue screen
    if (extra_button != BlackoutButton::None){
        Button b_button;
        switch (extra_button){
        case BlackoutButton::L:
            b_button = BUTTON_L;
            break;
        case BlackoutButton::R:
            b_button = BUTTON_R;
            break;
        default:
            b_button = BUTTON_L;
        }
        Milliseconds blackout_wait = 3800ms; // wait for the copyright text to disappear
        Milliseconds blackout_delay = std::chrono::milliseconds(seed_delay) - blackout_wait;
        Milliseconds blackout_hold = 30000ms; // wait for leaves/flames to appear on the title screen. It's okay if this is held over the seed button press
        ssf_do_nothing(context, blackout_wait);
        ssf_press_button(context, b_button, blackout_delay, blackout_hold, 0ms);
    }else{
        pbf_wait(context, std::chrono::milliseconds(seed_delay));
    }

    // hold the specified button for a few seconds through the transition to the Continue Screen
    Button s_button;
    switch (seed_button){
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

void load_game_after_delay(ProControllerContext& context, uint64_t csf_delay){
    pbf_wait(context, std::chrono::milliseconds(csf_delay - 3000));
    pbf_press_button(context, BUTTON_A, 50ms, 1450ms);
    // skip recap
    pbf_press_button(context, BUTTON_B, 50ms, 2450ms);
    // need to later subtract 4000ms from delay to hit desired number of advances
}

void wait_with_teachy_tv(ProControllerContext& context, uint64_t teachy_delay){
    // open start menu -> bag -> key items -> Teachy TV -> use
    pbf_press_button(context, BUTTON_PLUS, 200ms, 300ms);
    pbf_move_left_joystick(context, {0, -1}, 200ms, 300ms);
    pbf_move_left_joystick(context, {0, -1}, 200ms, 300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 2300ms);
    pbf_move_left_joystick(context, {+1, 0}, 200ms, 2300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 367ms); // the "static fuzz" that adds RNG advances doesn't kick in right away
    pbf_wait(context, std::chrono::milliseconds(teachy_delay));
    // close teachy tv -> close bag -> reset start menu cursor position - > close start menu
    pbf_press_button(context, BUTTON_B, 200ms, 2300ms);
    pbf_press_button(context, BUTTON_B, 200ms, 2300ms);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    pbf_press_button(context, BUTTON_B, 200ms, 300ms);
    // total non-teachy delay duration: 14067ms
}


void collect_starter_after_delay(ProControllerContext& context, uint64_t ingame_delay){
    // Advance through starter dialogue and wait on "really quite energetic!"
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(ingame_delay - STARTER_FIXED_DURATION));
    // Finish dialogue (hits the target advance)
    pbf_press_button(context, BUTTON_A, 200ms, 5800ms);
    // Decline nickname
    pbf_mash_button(context, BUTTON_B, 2500ms);
    // Advance through rival choice
    pbf_mash_button(context, BUTTON_B, 5000ms);
    context.wait_for_all_requests();
}

void collect_magikarp_after_delay(ProControllerContext& context, uint64_t ingame_delay, Language language){
    // Advance through starter dialogue and wait on YES/NO
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    if (language != Language::French){
        pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    }
    if (language == Language::Japanese){
        pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    }
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(ingame_delay - magikarp_fixed_duration(language)));
    // Finish dialogue (hits the target advance)
    pbf_press_button(context, BUTTON_A, 200ms, 3800ms);
    // Decline nickname
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
}

void collect_hitmon_after_delay(ProControllerContext& context, uint64_t ingame_delay){
    // One dialog before accepting
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(ingame_delay - HITMON_FIXED_DURATION));
    // Confirm selection
    pbf_press_button(context, BUTTON_A, 200ms, 1800ms);
    // Decline nickname
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
}

void collect_eevee_after_delay(ProControllerContext& context, uint64_t ingame_delay){
    // No dialogue to advance through -- just wait
    pbf_wait(context, std::chrono::milliseconds(ingame_delay - EEVEE_FIXED_DURATION));
    // Interact with the pokeball
    pbf_press_button(context, BUTTON_A, 200ms, 3800ms);
    // Decline nickname
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
}

void collect_lapras_after_delay(ProControllerContext& context, uint64_t ingame_delay, Language language){
    // 3 dialog presses
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    if (language != Language::French){
        pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    }
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(ingame_delay - lapras_fixed_duration(language)));
    // Accept Lapras on target frame
    pbf_press_button(context, BUTTON_A, 200ms, 3800ms);
    // Decline nickname and exit dialog
    pbf_mash_button(context, BUTTON_B, 7500ms);
    context.wait_for_all_requests();
}

void collect_fossil_after_delay(ProControllerContext& context, uint64_t ingame_delay){
    // 2 dialog presses
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(ingame_delay - FOSSIL_FIXED_DURATION));
    // Advance dialog on target frame
    pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
    // Decline nickname
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
}

void collect_gamecorner_after_delay(ProControllerContext& context, uint64_t ingame_delay, int SLOT){
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
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(ingame_delay - GAMECORNER_FIXED_DURATION));
    // confirm prize
    pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
    // decline nickname
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
}

void collect_togepi_egg_after_delay(ProControllerContext& context, uint64_t ingame_delay){
    // 6 dialog presses
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(ingame_delay - TOGEPI_FIXED_DURATION));
    // accept egg
    pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
    // exit dialogue
    pbf_mash_button(context, BUTTON_B, 2500ms);
    context.wait_for_all_requests();    
}

void collect_preapproved_togepi_egg_after_delay(ProControllerContext& context, uint64_t ingame_delay){
    // wait with the start menu open to avoid overworld stuff causing extra advances
    pbf_press_button(context, BUTTON_PLUS, 200ms, 300ms);
    pbf_wait(context, std::chrono::milliseconds(ingame_delay - TOGEPI_FAST_FIXED_DURATION));
    pbf_press_button(context, BUTTON_B, 200ms, 300ms);
    // accept egg
    pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
    // exit dialogue
    pbf_mash_button(context, BUTTON_B, 2500ms);
}

void trigger_held_daycare_egg_after_delay(ProControllerContext& context, uint64_t ingame_delay){
    // No dialogue to advance through -- just wait
    pbf_wait(context, std::chrono::milliseconds(ingame_delay - EGG_HELD_FIXED_DURATION));
    // Trigger the encounter (WALK LEFT)
    pbf_move_left_joystick(context, {-1, 0}, 200ms, 800ms);
    context.wait_for_all_requests();
}

void collect_daycare_egg_after_delay(ProControllerContext& context, uint64_t ingame_delay){
    // wait with the start menu open to avoid overworld stuff causing extra advances
    pbf_press_button(context, BUTTON_PLUS, 200ms, 300ms);
    pbf_wait(context, std::chrono::milliseconds(ingame_delay - EGG_PICKUP_FIXED_DURATION));
    pbf_press_button(context, BUTTON_B, 200ms, 300ms);
    // 5 dialog presses
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 800ms);
    // accept egg
    pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
    // exit dialogue
    pbf_mash_button(context, BUTTON_B, 2500ms);
    context.wait_for_all_requests();    
}

void encounter_static_after_delay(ProControllerContext& context, uint64_t ingame_delay){
    // No dialogue to advance through -- just wait in the start menu (avoids extra RNG advances by boulders Mt Ember and Seafoam Islands)
    pbf_press_button(context, BUTTON_PLUS, 200ms, 300ms);
    pbf_wait(context, std::chrono::milliseconds(ingame_delay - STATIC_FIXED_DURATION));
    pbf_press_button(context, BUTTON_B, 200ms, 300ms);
    // Interact with the static encounter
    pbf_press_button(context, BUTTON_A, 200ms, 800ms);
    pbf_mash_button(context, BUTTON_A, 1000ms); // finishes dialog for the legendary birds
    context.wait_for_all_requests();
}

void encounter_snorlax_after_delay(ProControllerContext& context, uint64_t ingame_delay){
    // Interact with Snorlax, YES to PokeFlute, wait on "woke up!"
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 9800ms); // PokeFlute tune
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(ingame_delay - SNORLAX_FIXED_DURATION));
    pbf_press_button(context, BUTTON_A, 200ms, 200ms); 
    context.wait_for_all_requests();
}

void encounter_mewtwo_after_delay(ProControllerContext& context, uint64_t ingame_delay){
    // one dialogue before the encounter happens
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(ingame_delay - MEWTWO_FIXED_DURATION));
    // Initiate encounter
    pbf_press_button(context, BUTTON_A, 200ms, 200ms); 
    context.wait_for_all_requests();
}

void encounter_hooh_after_delay(ProControllerContext& context, uint64_t ingame_delay){
    // No dialogue to advance through -- just wait
    pbf_wait(context, std::chrono::milliseconds(ingame_delay - HOOH_FIXED_DURATION));
    // Trigger the encounter (WALK UP)
    pbf_move_left_joystick(context, {0, +1}, 800ms, 700ms);
    context.wait_for_all_requests();
}

void encounter_hypno_after_delay(ProControllerContext& context, uint64_t ingame_delay){
    // 5 dialog advances, with the 5th needing some extra time
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 2300ms);
    // Wait after the 6th
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(ingame_delay - HYPNO_FIXED_DURATION));
    // Initiate encounter
    pbf_press_button(context, BUTTON_A, 200ms, 200ms); 
    context.wait_for_all_requests();
}

void use_sweet_scent(ProControllerContext& context, uint64_t ingame_delay, bool safari_zone = false){
    // navigate to last party slot
    pbf_press_button(context, BUTTON_PLUS, 200ms, 300ms);
    pbf_move_left_joystick(context, {0, -1}, 200ms, 300ms);
    if (safari_zone) { // there is an extra menu option at the top of the start menu
        pbf_move_left_joystick(context, {0, -1}, 200ms, 300ms); 
    }
    pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 300ms);
    // hover over Sweet Scent (2nd option, but maybe HMs could change this)
    pbf_move_left_joystick(context, {0, -1}, 200ms, std::chrono::milliseconds(ingame_delay - (safari_zone ? SAFARI_SWEET_SCENT_FIXED_DURATION : SWEET_SCENT_FIXED_DURATION)));
    pbf_press_button(context, BUTTON_A, 200ms, 800ms);
    context.wait_for_all_requests();
}

void use_rock_smash(ProControllerContext& context, uint64_t ingame_delay){
    // three button presses
    pbf_press_button(context, BUTTON_A, 200ms, 1800ms);
    pbf_press_button(context, BUTTON_A, 200ms, 300ms);
    pbf_wait(context, std::chrono::milliseconds(ingame_delay - ROCK_SMASH_FIXED_DURATION));
    pbf_press_button(context, BUTTON_A, 200ms, 800ms);
    context.wait_for_all_requests();
}

void use_registered_fishing_rod(ProControllerContext& context, uint64_t ingame_delay){
    uint32_t rng_wait = 50 * random_u32(0, 20); // helps avoid always hitting "Not even a nibble" (?)
    pbf_wait(context, std::chrono::milliseconds(rng_wait));
    pbf_press_button(context, BUTTON_MINUS, 200ms, std::chrono::milliseconds(ingame_delay - rng_wait - FISHING_FIXED_DURATION));
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
    // this last press is needed for German, but shouldn't throw off things off for other languages
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    // finish dialogue and automatically enter the Safari Zone
    pbf_press_button(context, BUTTON_A, 200ms, 4800ms);
    // total duration: 20000ms
}

void walk_to_safarizonefish(ProControllerContext& context){
    enter_safarizone(context); // 20000ms
    // walk from the entrance to the pond in the central area
    pbf_move_left_joystick(context, {0, +1}, 2200ms, 300ms);
    pbf_move_left_joystick(context, {+1, 0}, 1600ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 600ms, 300ms);
    // total duration: 25300ms
}

void walk_to_safarizonesurf(ProControllerContext& context){
    walk_to_safarizonefish(context); // 25300ms
    // start surfing
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 3300ms);
    // total duration: 31800ms
}

void walk_to_safarizonecenter(ProControllerContext& context){
    enter_safarizone(context); // 20000ms
    // walk from the entrance to the nearest grass
    pbf_move_left_joystick(context, {0, +1}, 460ms, 300ms);
    pbf_move_left_joystick(context, {-1, 0}, 1110ms, 300ms);
    // total duration: 22170ms
}

void walk_to_safarizoneeast(ProControllerContext& context){
    enter_safarizone(context); // 20000ms
    // walk from the entrance to the east area
    pbf_move_left_joystick(context, {0, +1}, 160ms, 300ms);
    pbf_move_left_joystick(context, {+1, 0}, 4400ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 3550ms, 300ms);
    // walk to the nearest grass
    pbf_move_left_joystick(context, {+1, 0}, 3600ms, 300ms);
    pbf_move_left_joystick(context, {0, -1}, 700ms, 300ms);
    pbf_move_left_joystick(context, {+1, 0}, 3450ms, 300ms);
    // total duration: 37660ms
}

void walk_to_safarizonenorth(ProControllerContext& context){
    walk_to_safarizonesurf(context); // 31800ms
    // from the pond to the grass in the north area
    pbf_move_left_joystick(context, {0, +1}, 2810ms, 300ms);
    pbf_move_left_joystick(context, {-1, 0}, 1530ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 1870ms, 300ms);
    // total duration: 38910ms
}

void walk_to_safarizonewest(ProControllerContext& context){
    walk_to_safarizonesurf(context); // 31800ms
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
    // total duration: 52930ms
}

void activate_roamer(ProControllerContext& context, Language language, uint64_t ingame_delay){
    uint64_t offset = roamer_fixed_duration(language);
    
    // a lot of dialogue while Celio inserts the Sapphire
    pbf_press_button(context, BUTTON_A, 200ms, 5300ms); // handed the sapphire to Celio
                                                        // "So this is the gem..."
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms); // "<Player>, you've gone through a lot..."
    if (language == Language::Japanese){                // extra dialog box for the above message
        pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    }
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms); // "You don't have to tell  me..."
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms); // "Thank you so much!"
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms); // "Now it's my turn..."
    pbf_press_button(context, BUTTON_A, 200ms, 3300ms); // "Okay, this is good..."
    pbf_press_button(context, BUTTON_A, 200ms, 5300ms); // "I did it! I linked up with LANETTE!"
    if (language == Language::Spanish){                 // "con AREDIA!" is pushed to a new line
        pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    }
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms); // "<Player!>! <Player>, I did it!"
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms); // "I've managed to link up with TRAINERS in the HOENN region!"
    if (language == Language::Spanish){                 // "de HOENN!" gets pushed to a new line
        pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    }
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms); // "Finally, the Network Machine..."
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms); // "<Player>, I owe it all to you!"
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(ingame_delay - offset)); 
                                                        // "Thanks to you, my dream came true..."
    pbf_press_button(context, BUTTON_A, 200ms, 800ms);  // finalize roamer and exit dialogue
    context.wait_for_all_requests();
}


struct IngameSequence{
    const char* name;
    uint64_t fixed_duration;   
    uint64_t min_wait;
    int64_t advances_offset;   
};

IngameSequence get_ingame_sequence(PokemonFRLG_RngTarget target, Language language){
    switch (target){
    case PokemonFRLG_RngTarget::starters:
        return {"Starters", STARTER_FIXED_DURATION, MIN_PRESS_GAP, DEFAULT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::magikarp:
        return {"Magikarp", magikarp_fixed_duration(language), MIN_PRESS_GAP, DEFAULT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::hitmonchan:
    case PokemonFRLG_RngTarget::hitmonlee:
    case PokemonFRLG_RngTarget::hitmon:
        return {"Hitmonchan/Hitmonlee", HITMON_FIXED_DURATION, MIN_PRESS_GAP, DEFAULT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::eevee:
        return {"Eevee", EEVEE_FIXED_DURATION, 0, DEFAULT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::lapras:
        return {"Lapras", lapras_fixed_duration(language), MIN_PRESS_GAP, DEFAULT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::omanyte:
    case PokemonFRLG_RngTarget::kabuto:
    case PokemonFRLG_RngTarget::aerodactyl:
    case PokemonFRLG_RngTarget::fossils:
        return {"Fossils", FOSSIL_FIXED_DURATION, MIN_PRESS_GAP, DEFAULT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::gamecornerabra:
    case PokemonFRLG_RngTarget::gamecornerclefairy:
    case PokemonFRLG_RngTarget::gamecornerdratinifr:
    case PokemonFRLG_RngTarget::gamecornerdratinilg:
    case PokemonFRLG_RngTarget::gamecornerscyther:
    case PokemonFRLG_RngTarget::gamecornerpinsir:
    case PokemonFRLG_RngTarget::gamecornerporygon:
        return {"Game Corner", GAMECORNER_FIXED_DURATION, MIN_PRESS_GAP, DEFAULT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::togepi:
        return {"Togepi", TOGEPI_FIXED_DURATION, MIN_PRESS_GAP, DEFAULT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::togepifast:
        return {"Togepi (pre-approved)", TOGEPI_FAST_FIXED_DURATION, 0, DEFAULT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::eggheld:
        return {"Held Frame", EGG_HELD_FIXED_DURATION, 0, 135};
    case PokemonFRLG_RngTarget::eggpickup:
        return {"Pickup Frame", EGG_PICKUP_FIXED_DURATION, 0, -246};
    case PokemonFRLG_RngTarget::electrode:
    case PokemonFRLG_RngTarget::articuno:
    case PokemonFRLG_RngTarget::zapdos:
    case PokemonFRLG_RngTarget::moltres:
    case PokemonFRLG_RngTarget::lugia:
    case PokemonFRLG_RngTarget::deoxys_attack:
    case PokemonFRLG_RngTarget::deoxys_defense:
    case PokemonFRLG_RngTarget::staticencounter:
        return {"Static Encounter", STATIC_FIXED_DURATION, 0, DEFAULT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::snorlax:
        return {"Snorlax", SNORLAX_FIXED_DURATION, MIN_PRESS_GAP, DEFAULT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::mewtwo:
        return {"Mewtwo", MEWTWO_FIXED_DURATION, MIN_PRESS_GAP, DEFAULT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::hooh:
        return {"Ho-oh", HOOH_FIXED_DURATION, 0, DEFAULT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::hypno:
        return {"Hypno", HYPNO_FIXED_DURATION, MIN_PRESS_GAP, DEFAULT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::sweetscent:
        return {"Sweet Scent", SWEET_SCENT_FIXED_DURATION, MIN_PRESS_GAP, SWEET_SCENT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::rocksmash:
        return {"Rock Smash", ROCK_SMASH_FIXED_DURATION, 0, SWEET_SCENT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::fishing:
        return {"Fishing", FISHING_FIXED_DURATION + FISHING_MAX_RNG_WAIT, MIN_PRESS_GAP, DEFAULT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::safarizonecenter:
        return {"Safari Zone Center", SAFARI_CENTER_WALK_DURATION + SAFARI_SWEET_SCENT_FIXED_DURATION, MIN_PRESS_GAP, SWEET_SCENT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::safarizoneeast:
        return {"Safari Zone East", SAFARI_EAST_WALK_DURATION + SAFARI_SWEET_SCENT_FIXED_DURATION, MIN_PRESS_GAP, SWEET_SCENT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::safarizonenorth:
        return {"Safari Zone North", SAFARI_NORTH_WALK_DURATION + SAFARI_SWEET_SCENT_FIXED_DURATION, MIN_PRESS_GAP, SWEET_SCENT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::safarizonewest:
        return {"Safari Zone West", SAFARI_WEST_WALK_DURATION + SAFARI_SWEET_SCENT_FIXED_DURATION, MIN_PRESS_GAP, SWEET_SCENT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::safarizonesurf:
        return {"Safari Zone Surfing", SAFARI_SURF_WALK_DURATION + SAFARI_SWEET_SCENT_FIXED_DURATION, MIN_PRESS_GAP, SWEET_SCENT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::safarizonefish:
        return {"Safari Zone Fishing", SAFARI_FISH_WALK_DURATION + FISHING_FIXED_DURATION + FISHING_MAX_RNG_WAIT, MIN_PRESS_GAP, DEFAULT_ADVANCES_OFFSET};
    case PokemonFRLG_RngTarget::raikou:
    case PokemonFRLG_RngTarget::entei:
    case PokemonFRLG_RngTarget::suicune:
    case PokemonFRLG_RngTarget::roaming:
        return {"Roaming", roamer_fixed_duration(language), MIN_PRESS_GAP, DEFAULT_ADVANCES_OFFSET};
    }
    return {nullptr, 0, 0, 0};
}

int64_t fixed_advances_offset(PokemonFRLG_RngTarget target){
    return get_ingame_sequence(target, Language::None).advances_offset;
}

void check_timings(
    ConsoleHandle& console,
    PokemonFRLG_RngTarget TARGET,
    const RngTimings& timings,
    Language language,
    uint64_t ingame_advances,
    uint64_t continue_screen_frames
){
    if (timings.csf_delay < 3200){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::NO_ERROR_REPORT,
            "The Continue Screen delay cannot be less than 3200ms (192 advances). Check your Continue Screen calibration.",
            console
        );
    }
    if (timings.seed_delay < 29650){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::NO_ERROR_REPORT,
            "The title screen delay cannot be less than 30s. Check your seed delay and calibration.",
            console
        );
    }

    IngameSequence sequence = get_ingame_sequence(TARGET, language);
    if (sequence.name == nullptr){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::NO_ERROR_REPORT,
            "RNG target not recognized. Please report this as a bug.",
            console
        );
    }

    uint64_t min_delay = sequence.fixed_duration + sequence.min_wait;
    if (timings.ingame_delay >= min_delay){
        return;
    }

    std::string message = std::string(sequence.name) + ": the in-game delay (" + std::to_string(timings.ingame_delay)
        + "ms) cannot be less than " + std::to_string(min_delay) + "ms. ";
    if (timings.teachy_delay > 0){
        // extra advances would be spent in the Teachy TV rather than added to the in-game delay
        message += "The Teachy TV doesn't leave enough time for this target. Disable the Teachy TV or pick a new target.";
    }else{
        // each frame of in-game delay is worth 2 advances. Round up to the nearest 10 advances.
        uint64_t missing_advances = uint64_t(std::ceil((min_delay - timings.ingame_delay) * 2 / FRLG_FRAME_DURATION));
        uint64_t min_ingame_advances = (ingame_advances + missing_advances + 9) / 10 * 10;
        message += "With your current calibration, this target needs at least " + std::to_string(min_ingame_advances)
            + " in-game advances (" + std::to_string(min_ingame_advances + continue_screen_frames)
            + " total). Check your advances and calibration or pick a new target.";
    }
    OperationFailedExceptionWithScreenshot::fire(
        ErrorReportMode::NO_ERROR_REPORT,
        message,
        console
    );
}

void perform_blind_sequence(
    ProControllerContext& context, 
    Language language,
    PokemonFRLG_RngTarget target,
    SeedButton seed_button,
    BlackoutButton extra_button,
    const RngTimings& timings,
    bool safari_zone,
    ConsoleType console_type
){
    pbf_press_button(context, BUTTON_A, 80ms, 0ms); // start the game from the Home screen
    set_seed_after_delay(context, seed_button, extra_button, timings.seed_delay, console_type);
    load_game_after_delay(context, timings.csf_delay);
    if (timings.teachy_delay > 0){
        wait_with_teachy_tv(context, timings.teachy_delay);
    }

    uint64_t modified_ingame_delay;
    switch (target){
    case PokemonFRLG_RngTarget::starters:
        collect_starter_after_delay(context, timings.ingame_delay);
        return;
    case PokemonFRLG_RngTarget::magikarp:
        collect_magikarp_after_delay(context, timings.ingame_delay, language);
        return;
    case PokemonFRLG_RngTarget::hitmonchan:
    case PokemonFRLG_RngTarget::hitmonlee:
    case PokemonFRLG_RngTarget::hitmon:
        collect_hitmon_after_delay(context, timings.ingame_delay);
        return;
    case PokemonFRLG_RngTarget::eevee:
        collect_eevee_after_delay(context, timings.ingame_delay);
        return;
    case PokemonFRLG_RngTarget::lapras:
        collect_lapras_after_delay(context, timings.ingame_delay, language);
        return;
    case PokemonFRLG_RngTarget::omanyte:
    case PokemonFRLG_RngTarget::kabuto:
    case PokemonFRLG_RngTarget::aerodactyl:
    case PokemonFRLG_RngTarget::fossils:
        collect_fossil_after_delay(context, timings.ingame_delay);
        return;
    case PokemonFRLG_RngTarget::gamecornerabra:
        collect_gamecorner_after_delay(context, timings.ingame_delay, 0);
        return;
    case PokemonFRLG_RngTarget::gamecornerclefairy:
        collect_gamecorner_after_delay(context, timings.ingame_delay, 1);
        return;
    case PokemonFRLG_RngTarget::gamecornerdratinifr:
    case PokemonFRLG_RngTarget::gamecornerpinsir:
        collect_gamecorner_after_delay(context, timings.ingame_delay, 2);
        return;
    case PokemonFRLG_RngTarget::gamecornerscyther:
    case PokemonFRLG_RngTarget::gamecornerdratinilg:
        collect_gamecorner_after_delay(context, timings.ingame_delay, 3);
        return;
    case PokemonFRLG_RngTarget::gamecornerporygon:
        collect_gamecorner_after_delay(context, timings.ingame_delay, 4);
        return;
    case PokemonFRLG_RngTarget::togepi:
        collect_togepi_egg_after_delay(context, timings.ingame_delay);
        return;
    case PokemonFRLG_RngTarget::togepifast:
        collect_preapproved_togepi_egg_after_delay(context, timings.ingame_delay);
        return;
    case PokemonFRLG_RngTarget::eggheld:
        trigger_held_daycare_egg_after_delay(context, timings.ingame_delay);
        return;
    case PokemonFRLG_RngTarget::eggpickup:
        collect_daycare_egg_after_delay(context, timings.ingame_delay);
        return;
    case PokemonFRLG_RngTarget::electrode:
    case PokemonFRLG_RngTarget::articuno:
    case PokemonFRLG_RngTarget::zapdos:
    case PokemonFRLG_RngTarget::moltres:
    case PokemonFRLG_RngTarget::lugia:
    case PokemonFRLG_RngTarget::deoxys_attack:
    case PokemonFRLG_RngTarget::deoxys_defense:
    case PokemonFRLG_RngTarget::staticencounter:
        encounter_static_after_delay(context, timings.ingame_delay);
        return;
    case PokemonFRLG_RngTarget::snorlax:
        encounter_snorlax_after_delay(context, timings.ingame_delay);
        return;
    case PokemonFRLG_RngTarget::mewtwo:
        encounter_mewtwo_after_delay(context, timings.ingame_delay);
        return;
    case PokemonFRLG_RngTarget::hooh:
        encounter_hooh_after_delay(context, timings.ingame_delay);
        return;
    case PokemonFRLG_RngTarget::hypno:
        encounter_hypno_after_delay(context, timings.ingame_delay);
        return;
    case PokemonFRLG_RngTarget::sweetscent:
        use_sweet_scent(context, timings.ingame_delay, safari_zone);
        return;
    case PokemonFRLG_RngTarget::rocksmash:
        use_rock_smash(context, timings.ingame_delay);
        return;
    case PokemonFRLG_RngTarget::fishing:
        use_registered_fishing_rod(context, timings.ingame_delay);
        return;
    case PokemonFRLG_RngTarget::safarizonecenter:
        modified_ingame_delay = timings.ingame_delay - SAFARI_CENTER_WALK_DURATION;
        walk_to_safarizonecenter(context);
        use_sweet_scent(context, modified_ingame_delay, true);
        return;
    case PokemonFRLG_RngTarget::safarizoneeast:
        modified_ingame_delay = timings.ingame_delay - SAFARI_EAST_WALK_DURATION;
        walk_to_safarizoneeast(context);
        use_sweet_scent(context, modified_ingame_delay, true);
        return;
    case PokemonFRLG_RngTarget::safarizonenorth:
        modified_ingame_delay = timings.ingame_delay - SAFARI_NORTH_WALK_DURATION;
        walk_to_safarizonenorth(context);
        use_sweet_scent(context, modified_ingame_delay, true);
        return;
    case PokemonFRLG_RngTarget::safarizonewest:
        modified_ingame_delay = timings.ingame_delay - SAFARI_WEST_WALK_DURATION;
        walk_to_safarizonewest(context);
        use_sweet_scent(context, modified_ingame_delay, true);
        return;
    case PokemonFRLG_RngTarget::safarizonesurf:
        modified_ingame_delay = timings.ingame_delay - SAFARI_SURF_WALK_DURATION;
        walk_to_safarizonesurf(context);
        use_sweet_scent(context, modified_ingame_delay, true);
        return;
    case PokemonFRLG_RngTarget::safarizonefish:
        modified_ingame_delay = timings.ingame_delay - SAFARI_FISH_WALK_DURATION;
        walk_to_safarizonefish(context);
        use_registered_fishing_rod(context, modified_ingame_delay);
        return;
    case PokemonFRLG_RngTarget::raikou:
    case PokemonFRLG_RngTarget::entei:
    case PokemonFRLG_RngTarget::suicune:
    case PokemonFRLG_RngTarget::roaming:
        activate_roamer(context, language, timings.ingame_delay);
    }
}

}
}
}