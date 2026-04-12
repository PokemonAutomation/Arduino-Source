/*  RNG Helper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "CommonTools/Random.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonFRLG/Inference/PokemonFRLG_SelectionArrowDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_ShinySymbolDetector.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/Programs/PokemonFRLG_StartMenuNavigation.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_RngHelper.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

RngHelper_Descriptor::RngHelper_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:RngHelper",
        Pokemon::STRING_POKEMON + " FRLG", "RNG Helper",
        "Programs/PokemonFRLG/RngHelper.html",
        "Soft reset with specific timings for hitting a target Seed and Frame for RNG manipulation.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct RngHelper_Descriptor::Stats : public StatsTracker{
    Stats()
        : resets(m_stats["Resets"])
        , shinies(m_stats["Shinies"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> RngHelper_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

RngHelper::RngHelper()
    : TARGET(
        "<b>Target:</b><br>",
        {
            {Target::starters, "starters", "Bulbasaur / Squirtle / Charmander"},
            {Target::magikarp, "magikarp", "Magikarp"},
            {Target::hitmon, "hitmon", "Hitmonlee / Hitmonchan"},
            {Target::eevee, "eevee", "Eevee"},
            {Target::lapras, "lapras", "Lapras"},
            {Target::fossils, "fossils", "Omanyte / Kabuto / Aerodactyl"},
            {Target::gamecornerabra, "gamecornerabra", "Game Corner Abra"},
            {Target::gamecornerclefairy, "gamecornerclefairy", "Game Corner Clefairy"},
            {Target::gamecornerdratini, "gamecornerdratini", "Game Corner Dratini"},
            {Target::gamecornerbug, "gamecornerbug", "Game Corner Bug (Scyther / Pinsir)"},
            {Target::gamecornerporygon, "gamecornerporygon", "Game Corner Porygon"},
            {Target::togepi, "togepi", "Togepi"},
            {Target::staticencounter, "staticencounter", "Static Overworld Encounters"},
            {Target::snorlax, "snorlax", "Snorlax"},
            {Target::mewtwo, "mewtwo", "Mewtwo"},
            {Target::hooh, "hooh", "Ho-oh"},
            {Target::hypno, "berryforesthypno", "Berry Forest Hypno"},
            {Target::sweetscent, "sweetscent", "Sweet Scent"},
            {Target::fishing, "fishing", "Fishing"},
            {Target::safarizonecenter, "safarizonecenter", "Safari Zone Center (Sweet Scent)"},
            {Target::safarizoneeast, "safarizoneeast", "Safari Zone East (Sweet Scent)"},
            {Target::safarizonenorth, "safarizonenorth", "Safari Zone North (Sweet Scent)"},
            {Target::safarizonewest, "safarizonewest", "Safari Zone West (Sweet Scent)"},
            {Target::safarizonesurf, "safarizonesurf", "Safari Zone Surfing"},
            {Target::safarizonefish, "safarizonefish", "Safari Zone Fishing"},
            // {Target::roaming, "roaming", "Roaming Legendaries"}
        },
        LockMode::LOCK_WHILE_RUNNING,
        Target::starters
    )    
    , NUM_RESETS(
        "<b>Max Resets:</b><br>",
        LockMode::UNLOCK_WHILE_RUNNING,
        1, 0 // default, min
    )
    , SEED_DELAY(
        "<b>Seed Delay Time (ms):</b><br>The delay between starting the game and advancing past the title screen. Set this to match your target seed.",
        LockMode::LOCK_WHILE_RUNNING,
        35000, 28000 // default, min
    )
    , SEED_CALIBRATION(
         "<b>Seed Calibration (ms):</b><br>Modifies the seed delay time.",
        LockMode::UNLOCK_WHILE_RUNNING,
        0  // default
    )
    , LOAD_ADVANCES(
        "<b>Load Screen Advances (frames):</b><br>The number of frames to advance before loading the game.<br>These pass at the \"normal\" rate compared to other consoles.",
        LockMode::LOCK_WHILE_RUNNING,
        1000, 192 // default, min
    )
    , LOAD_CALIBRATION(
        "<b>Load Screen Advances Calibration (frames):</b><br>A \"fine adjustment\" that modifies the frame advances passed on the load screen.<br>",
        LockMode::UNLOCK_WHILE_RUNNING,
        0 // default
    )
    , INGAME_ADVANCES(
        "<b>In-Game Advances (frames):</b><br>The number of frames to advance before triggering the gift/encounter.<br>These pass at double the rate compared to other consoles, where every 2nd frame is skipped.<br><i>Warning: this needs to be long enough to accomodate all in-game button presses prior to the gift/encounter</i>",
        LockMode::LOCK_WHILE_RUNNING,
        12345, 480 // default, min
    )
    , INGAME_CALIBRATION(
        "<b>In-Game Advances Calibration (frames):</b><br>A \"coarse adjustment\" that modifies the frame advances passed after loading the game.<br>",
        LockMode::UNLOCK_WHILE_RUNNING,
        0 // default
    )
    , USE_COPYRIGHT_TEXT(
        "<b>Detect Copyright Text:</b><br>Start the seed timer only after detecting the copyright text. Can be helpful for improving seed consistency.",
        LockMode::LOCK_WHILE_RUNNING,
        true // default
    )
    , USE_TEACHY_TV(
        "<b>Use Teachy TV:</b><br>Opens the Teachy TV to quickly advance in-game frames at 313x speed.<br><i>Warning: can result in larger misses.</i>",
        LockMode::LOCK_WHILE_RUNNING,
        false // default
    )
    , TAKE_VIDEO(
        "<b>Take Video:</b><br>Record a video when the shiny is found.", 
        LockMode::LOCK_WHILE_RUNNING, 
        true // default
    )
    , GO_HOME_WHEN_DONE(true)
    , NOTIFICATION_SHINY(
        "Shiny found",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_SHINY,
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(TARGET);
    PA_ADD_OPTION(NUM_RESETS);
    PA_ADD_OPTION(SEED_DELAY);
    PA_ADD_OPTION(SEED_CALIBRATION);
    PA_ADD_OPTION(LOAD_ADVANCES);
    PA_ADD_OPTION(LOAD_CALIBRATION);
    PA_ADD_OPTION(INGAME_ADVANCES);
    PA_ADD_OPTION(INGAME_CALIBRATION);
    PA_ADD_OPTION(USE_COPYRIGHT_TEXT);
    PA_ADD_OPTION(USE_TEACHY_TV);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

namespace{

void hard_reset(ProControllerContext& context){
    // close the game
    pbf_press_button(context, BUTTON_HOME, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_Y, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
    // press A to select game
    pbf_press_button(context, BUTTON_A, 200ms, 2300ms);
    // press A to select profile and launch the game
    pbf_press_button(context, BUTTON_A, 50ms, 150ms);
    // return to HOME menu and wait a moment
    pbf_press_button(context, BUTTON_HOME, 125ms, 2000ms);
    // open game
    pbf_press_button(context, BUTTON_A, 125ms, 0ms);
}


uint64_t wait_for_copyright_text(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // wait for copyright text to appear
    BlackScreenWatcher black_screen(COLOR_RED);
    context.wait_for_all_requests();
    WallClock start_time = current_time(); // immediately (more or less) after the button press to enter the game
    int black_ret = wait_until(
        env.console, context, 10000ms,
        {black_screen},
        1ms
    );
    if (black_ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Black screen not detected within 10 seconds of starting game.",
            env.console
        );
    }
    BlackScreenOverWatcher copyright_detected(COLOR_RED);
    int ret = wait_until(
        env.console, context, 10000ms,
        {copyright_detected },
        1ms
    );
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Black screen detected for more than 10 seconds after starting game.",
            env.console
        );
    }
    auto elapsed = current_time() - start_time; // when the copyright text appears
    return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
}

void set_seed_after_delay(ProControllerContext& context, SimpleIntegerOption<uint64_t>& SEED_DELAY,  SimpleIntegerOption<int64_t>& SEED_CALIBRATION, int64_t& FIXED_SEED_OFFSET){
    // wait on title screen for the specified delay
    pbf_wait(context, std::chrono::milliseconds(SEED_DELAY + SEED_CALIBRATION + FIXED_SEED_OFFSET));
    // hold A for a few seconds through the transition to the load screen
    pbf_press_button(context, BUTTON_A, 3000ms, 0ms);
}

void load_game_after_delay(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const uint64_t& LOAD_DELAY){
        if (LOAD_DELAY < 3200){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "The load screen delay cannot be less than 3200ms (192 frames). Check your load screen calibration.",
            env.console
        );
    }
    pbf_wait(context, std::chrono::milliseconds(LOAD_DELAY - 3000));
    pbf_press_button(context, BUTTON_A, 33ms, 1467ms);
    // skip recap
    pbf_press_button(context, BUTTON_B, 33ms, 2467ms);
    // need to later subtract 4000ms from delay to hit desired number of advances
}

void wait_with_teachy_tv(ProControllerContext& context, const uint64_t& TEACHY_DELAY){
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
    // if used in the Safari Zone: 14200ms
}

void collect_starter_after_delay(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const uint64_t& INGAME_DELAY){
    if (INGAME_DELAY < 7500){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Starters: the in-game delay cannot be less than 7500ms (900 frames). Check your in-game advances and calibration.",
            env.console
        );
    }
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
    env.log("Starter collected.");
}

void collect_magikarp_after_delay(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const uint64_t& INGAME_DELAY){
    if (INGAME_DELAY < 7500){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Magikarp: the in-game delay cannot be less than 7500ms (900 frames). Check your in-game advances and calibration.",
            env.console
        );
    }
    // Advance through starter dialogue and wait on YES/NO
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(INGAME_DELAY - 7200)); // 4000ms + 3000ms + 200ms
    // Finish dialogue (hits the target advance)
    pbf_press_button(context, BUTTON_A, 200ms, 3800ms);
    // Decline nickname
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
    env.log("Magikarp collected.");
}

void collect_hitmon_after_delay(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const uint64_t& INGAME_DELAY){
    if (INGAME_DELAY < 4500){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Hitmonchan/Hitmonlee: the in-game delay cannot be less than 4500ms (540 frames). Check your in-game advances and calibration.",
            env.console
        );
    }
    // One dialog before accepting
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(INGAME_DELAY - 4200)); // 4000ms + 200ms
    // Confirm selection
    pbf_press_button(context, BUTTON_A, 200ms, 1800ms);
    // Decline nickname
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
    env.log("Hitmonchan/Hitmonlee collected.");
}

void collect_eevee_after_delay(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const uint64_t& INGAME_DELAY){
    if (INGAME_DELAY < 4000){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Eevee: the in-game delay cannot be less than 4000ms (480 frames). Check your in-game advances and calibration.",
            env.console
        );
    }
    // No dialogue to advance through -- just wait
    pbf_wait(context, std::chrono::milliseconds(INGAME_DELAY - 4000));
    // Interact with the pokeball
    pbf_press_button(context, BUTTON_A, 200ms, 3800ms);
    // Decline nickname
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
    env.log("Eevee collected.");
}

void collect_lapras_after_delay(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const uint64_t& INGAME_DELAY){
    if (INGAME_DELAY < 7500){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Lapras: the in-game delay cannot be less than 7500ms (900 frames). Check your in-game advances and calibration.",
            env.console
        );
    }
    // 3 dialog presses
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(INGAME_DELAY - 7200)); // 4000ms + 3000ms + 200ms
    // Accept Lapras on target frame
    pbf_press_button(context, BUTTON_A, 200ms, 3800ms);
    // Decline nickname and exit dialog
    pbf_mash_button(context, BUTTON_B, 7500ms);
    context.wait_for_all_requests();
    env.log("Lapras collected.");
}

void collect_fossil_after_delay(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const uint64_t& INGAME_DELAY){
    if (INGAME_DELAY < 6000){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Fossils: the in-game delay cannot be less than 6000ms (720 frames). Check your in-game advances and calibration.",
            env.console
        );
    }
    // 2 dialog presses
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(INGAME_DELAY - 5700)); // 4000ms + 1500ms + 200ms
    // Advance dialog on target frame
    pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
    // Decline nickname
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
    env.log("Fossil Pokemon collected.");
}

void collect_gamecorner_after_delay(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const uint64_t& INGAME_DELAY, int SLOT){
    if (INGAME_DELAY < 8500){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Game Corner: the in-game delay cannot be less than 8500ms (1020 frames). Check your in-game advances and calibration.",
            env.console
        );
    }
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
    env.log("Game corner prize collected.");
}

void collect_togepi_egg_after_delay(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const uint64_t& INGAME_DELAY){
    if (INGAME_DELAY < 12000) {
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Togepi: the in-game delay cannot be less than 12000ms (1440 frames). Check your in-game advances and calibration.",
            env.console
        );
    }
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
    env.log("Togepi egg collected.");
}

void encounter_static_after_delay(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const uint64_t& INGAME_DELAY){
    if (INGAME_DELAY < 5000){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Static Encounter: the in-game delay cannot be less than 5000ms (600 frames). Check your in-game advances and calibration.",
            env.console
        );
    }
    // No dialogue to advance through -- just wait in the start menu (avoids extra RNG advances by boulders Mt Ember and Seafoam Islands)
    pbf_press_button(context, BUTTON_PLUS, 200ms, 300ms);
    pbf_wait(context, std::chrono::milliseconds(INGAME_DELAY - 5000)); // 4000ms + 1000ms
    pbf_press_button(context, BUTTON_B, 200ms, 300ms);
    // Interact with the static encounter
    pbf_press_button(context, BUTTON_A, 200ms, 800ms);
    pbf_mash_button(context, BUTTON_A, 1000ms); // finishes dialog for the legendary birds
    context.wait_for_all_requests();
    env.log("Static encounter started.");
}

void encounter_snorlax_after_delay(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const uint64_t& INGAME_DELAY){
    if (INGAME_DELAY < 16000){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Snorlax: the in-game delay cannot be less than 16000ms (1920 frames). Check your in-game advances and calibration.",
            env.console
        );
    }
    // Interact with Snorlax, YES to PokeFlute, wait on "woke up!"
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 9800ms); // PokeFlute tune
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(INGAME_DELAY - 15700)); // 4000ms + 1500ms + 10000ms + 200ms
    pbf_press_button(context, BUTTON_A, 200ms, 200ms); 
    context.wait_for_all_requests();
    env.log("Snorlax encounter started.");
}

void encounter_mewtwo_after_delay(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const uint64_t& INGAME_DELAY){
    if (INGAME_DELAY < 4500){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Mewtwo: the in-game delay cannot be less than 4500ms (540 frames). Check your in-game advances and calibration.",
            env.console
        );
    }
    // one dialogue before the encounter happens
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(INGAME_DELAY - 4200)); // 4000ms + 200ms
    // Initiate encounter
    pbf_press_button(context, BUTTON_A, 200ms, 200ms); 
    context.wait_for_all_requests();
    env.log("Hypno encounter started.");
}

void encounter_hooh_after_delay(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const uint64_t& INGAME_DELAY){
    if (INGAME_DELAY < 4000){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Ho-oh: the in-game delay cannot be less than 4000ms (480 frames). Check your in-game advances and calibration.",
            env.console
        );
    }
    // No dialogue to advance through -- just wait
    pbf_wait(context, std::chrono::milliseconds(INGAME_DELAY - 4000));
    // Trigger the encounter (WALK UP)
    pbf_move_left_joystick(context, {0, +1}, 800ms, 700ms);
    context.wait_for_all_requests();
    env.log("Ho-oh encounter started.");
}

void encounter_hypno_after_delay(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const uint64_t& INGAME_DELAY){
    if (INGAME_DELAY < 13000){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Hypno: the in-game delay cannot be less than 13000ms (1560 frames). Check your in-game advances and calibration.",
            env.console
        );
    }
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
    env.log("Hypno encounter started.");
}

void open_party_menu_from_overworld(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    pbf_press_button(context, BUTTON_PLUS, 200ms, 800ms);
    int ret = move_cursor_to_position(env.console, context, SelectionArrowPositionStartMenu::POKEMON);
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to navigate to POKEMON on the Start menu.",
            env.console
        ); 
    }
    pbf_press_button(context, BUTTON_A, 200ms, 1800ms);
    context.wait_for_all_requests();
}

void use_sweet_scent(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const uint64_t& INGAME_DELAY, bool SAFARI_ZONE = false){
    if (!SAFARI_ZONE && INGAME_DELAY < 8500){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Sweet Scent: the in-game delay cannot be less than 8500ms (1020 frames). Check your in-game advances and calibration.",
            env.console
        );
    }else if (SAFARI_ZONE && INGAME_DELAY < 9500){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Sweet Scent: the in-game delay cannot be less than 9500ms (1140 frames). Check your in-game advances and calibration.",
            env.console
        );
    }
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
    env.log("Sweet Scent used.");
}

void use_registered_fishing_rod(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const uint64_t& INGAME_DELAY){
    if (INGAME_DELAY < 5500){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Fishing: the in-game delay cannot be less than 5500ms (660 frames). Check your in-game advances and calibration.",
            env.console
        );
    }
    // n
    uint32_t rng_wait = 50 * random_u32(0, 20); // helps avoid always hitting "Not even a nibble" (?)
    pbf_wait(context, std::chrono::milliseconds(rng_wait));
    pbf_press_button(context, BUTTON_MINUS, 200ms, std::chrono::milliseconds(INGAME_DELAY - rng_wait - 4200));
    pbf_press_button(context, BUTTON_A, 200ms, 800ms);
    context.wait_for_all_requests();
}

void go_to_starter_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // Navigate to summary (1st party slot)
    open_party_menu_from_overworld(env, context);
    pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
    pbf_press_button(context, BUTTON_A, 200ms, 2300ms);
}

bool shiny_check_starter_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    go_to_starter_summary(env, context);
    context.wait_for_all_requests();
    VideoSnapshot screen = env.console.video().snapshot();
    ShinySymbolDetector shiny_checker(COLOR_YELLOW);
    return shiny_checker.read(env.console.logger(), screen);
}

void go_to_last_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // navigate to summary (last party slot)
    open_party_menu_from_overworld(env, context);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    // open summary
    pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
    pbf_press_button(context, BUTTON_A, 200ms, 2300ms);
}

bool shiny_check_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    go_to_last_summary(env, context);
    context.wait_for_all_requests();
    VideoSnapshot screen = env.console.video().snapshot();
    ShinySymbolDetector shiny_checker(COLOR_YELLOW);
    return shiny_checker.read(env.console.logger(), screen);
}

void hatch_togepi_egg(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // assumes the player is already on a bike and that the nearby trainer has been defeated
    // cycle to the right
    pbf_move_left_joystick(context, {+1, 0}, 1000ms, 200ms);
    pbf_move_left_joystick(context, {-1, 0}, 100ms, 500ms);
    WhiteDialogWatcher egg_dialog(COLOR_RED);
    context.wait_for_all_requests();
    WallClock deadline = current_time() + 600s;
    env.log("Hatching Togepi egg...");
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [deadline](ProControllerContext& context) {
            // cycle back and forth
            while (current_time() < deadline){
                pbf_move_left_joystick(context, {-1, 0}, 400ms, 0ms);
                pbf_move_left_joystick(context, {+1, 0}, 400ms, 0ms); 
            }
        },
        { egg_dialog }
    );
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Togepi: failed to hatch egg within 10 minutes. Check your in-game setup.",
            env.console
        );
    }

    // watch hatching animation and decline nickname
    pbf_mash_button(context, BUTTON_B, 15000ms);
    context.wait_for_all_requests();
}

int watch_for_shiny_encounter(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    BlackScreenWatcher battle_entered(COLOR_RED);
    context.wait_for_all_requests();
    env.log("Wild encounter started.");
    int ret = wait_until(
        env.console, context, 10000ms,
        {battle_entered}
    );
    if (ret < 0){
        // OperationFailedException::fire(
        //     ErrorReport::SEND_ERROR_REPORT,
        //     "Failed to initiate encounter.",
        //     env.console
        // );
        return -1;
    }
    bool encounter_shiny = handle_encounter(env.console, context, false);
    return encounter_shiny ? 1 : 0;
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

} // namespace


void RngHelper::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    /*
    * Settings: Text Speed fast
    */

    RngHelper_Descriptor::Stats& stats = env.current_stats<RngHelper_Descriptor::Stats>();

    home_black_border_check(env.console, context);

    bool shiny_found = false;

    double FRAMERATE = 59.999977;       // FPS. from Dhruv (don't know original source)
    double FRAME_DURATION = 1000 / FRAMERATE;

    int64_t FIXED_SEED_OFFSET = USE_COPYRIGHT_TEXT ? -2048 : -800;  // milliseconds. approximate, might be console-specific (?)
    int64_t FIXED_ADVANCES_OFFSET = 0;                              // frames

    while (!shiny_found){
        double MODIFIED_INGAME_ADVANCES = INGAME_ADVANCES + FIXED_ADVANCES_OFFSET + INGAME_CALIBRATION;
        if (MODIFIED_INGAME_ADVANCES < 0) {
           OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "In-game advances cannot be negative. Check your in-game advances and calibration.",
                env.console
            ); 
        }
        uint64_t TEACHY_ADVANCES = 0;

        const bool SAFARI_ZONE = (TARGET == Target::safarizonecenter
            || TARGET == Target::safarizoneeast
            || TARGET == Target::safarizonenorth
            || TARGET == Target::safarizonewest
            || TARGET == Target::safarizonesurf
            || TARGET == Target::safarizonefish 
        );

        uint64_t TEACHY_TV_BUFFER = SAFARI_ZONE ? 12000 : 5000; // Safari zone targets need extra time to walk to the right position

        bool should_use_teachy_tv = USE_TEACHY_TV && MODIFIED_INGAME_ADVANCES > TEACHY_TV_BUFFER; // don't use Teachy TV for short in-game advance targets
        if (should_use_teachy_tv) {
            TEACHY_ADVANCES = uint64_t((int)std::floor((MODIFIED_INGAME_ADVANCES - TEACHY_TV_BUFFER) / 313) * 313);
        }

        const uint64_t LOAD_DELAY = uint64_t((LOAD_ADVANCES + LOAD_CALIBRATION) * FRAME_DURATION);
        const uint64_t TEACHY_DELAY = uint64_t(TEACHY_ADVANCES * FRAME_DURATION / 313);
        const uint64_t INGAME_DELAY = uint64_t((MODIFIED_INGAME_ADVANCES - TEACHY_ADVANCES) * FRAME_DURATION / 2) - (should_use_teachy_tv ? 13700 : 0);
        env.log("Load screen delay: " + std::to_string(LOAD_DELAY) + "ms");
        env.log("In-game delay: " + std::to_string(INGAME_DELAY) + "ms");
        env.log("Teachy TV delay: " + std::to_string(TEACHY_DELAY) + "ms");
        env.log("Total time: " + std::to_string(SEED_DELAY + SEED_CALIBRATION + FIXED_SEED_OFFSET + LOAD_DELAY + INGAME_DELAY + TEACHY_DELAY) + "ms");

        if (SEED_DELAY + SEED_CALIBRATION + FIXED_SEED_OFFSET < 28000){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "The title screen delay cannot be less than 28000ms. Check your seed calibration.",
                env.console
            );
        }

        hard_reset(context);

        if (USE_COPYRIGHT_TEXT) {
            uint64_t STARTUP_DELAY = wait_for_copyright_text(env, context);
            env.log("Startup delay: " + std::to_string(STARTUP_DELAY) + "ms");
        }

        set_seed_after_delay(context, SEED_DELAY, SEED_CALIBRATION, FIXED_SEED_OFFSET);
        load_game_after_delay(env, context, LOAD_DELAY);
        if (should_use_teachy_tv){
            wait_with_teachy_tv(context, TEACHY_DELAY);
        }

        int ret;
        uint64_t MODIFIED_INGAME_DELAY;
        switch (TARGET){
        case Target::starters:
            collect_starter_after_delay(env, context, INGAME_DELAY);
            shiny_found = shiny_check_starter_summary(env, context);
            break;
        case Target::magikarp:
            collect_magikarp_after_delay(env, context, INGAME_DELAY);
            shiny_found = shiny_check_summary(env, context);
            break;
        case Target::hitmon:
            collect_hitmon_after_delay(env, context, INGAME_DELAY);
            shiny_found = shiny_check_summary(env, context);
            break;
        case Target::eevee:
            collect_eevee_after_delay(env, context, INGAME_DELAY);
            shiny_found = shiny_check_summary(env, context);
            break;
        case Target::lapras:
            collect_lapras_after_delay(env, context, INGAME_DELAY);
            shiny_found = shiny_check_summary(env, context);
            break;
        case Target::fossils:
            collect_fossil_after_delay(env, context, INGAME_DELAY);
            shiny_found = shiny_check_summary(env, context);
            break;
        case Target::gamecornerabra:
            collect_gamecorner_after_delay(env, context, INGAME_DELAY, 0);
            shiny_found = shiny_check_summary(env, context);
            break;
        case Target::gamecornerclefairy:
            collect_gamecorner_after_delay(env, context, INGAME_DELAY, 1);
            shiny_found = shiny_check_summary(env, context);
            break;
        case Target::gamecornerdratini:
            collect_gamecorner_after_delay(env, context, INGAME_DELAY, 2);
            shiny_found = shiny_check_summary(env, context);
            break;
        case Target::gamecornerbug:
            collect_gamecorner_after_delay(env, context, INGAME_DELAY, 3);
            shiny_found = shiny_check_summary(env, context);
            break;
        case Target::gamecornerporygon:
            collect_gamecorner_after_delay(env, context, INGAME_DELAY, 4);
            shiny_found = shiny_check_summary(env, context);
            break;
        case Target::togepi:
            collect_togepi_egg_after_delay(env, context, INGAME_DELAY);
            hatch_togepi_egg(env, context);
            shiny_found = shiny_check_summary(env, context);
            break;
        case Target::staticencounter:
            encounter_static_after_delay(env, context, INGAME_DELAY);
            shiny_found = watch_for_shiny_encounter(env, context) == 1;
            break;
        case Target::snorlax:
            encounter_snorlax_after_delay(env, context, INGAME_DELAY);
            shiny_found = watch_for_shiny_encounter(env, context) == 1;
            break;
        case Target::mewtwo:
            encounter_mewtwo_after_delay(env, context, INGAME_DELAY);
            shiny_found = watch_for_shiny_encounter(env, context) == 1;
            break;
        case Target::hooh:
            encounter_hooh_after_delay(env, context, INGAME_DELAY);
            shiny_found = watch_for_shiny_encounter(env, context) == 1;
            break;
        case Target::hypno:
            encounter_hypno_after_delay(env, context, INGAME_DELAY);
            shiny_found = watch_for_shiny_encounter(env, context) == 1;
            break;
        case Target::sweetscent:
            use_sweet_scent(env, context, INGAME_DELAY);
            shiny_found = watch_for_shiny_encounter(env, context) == 1;
            break;
        case Target::fishing:
            use_registered_fishing_rod(env, context, INGAME_DELAY);
            ret = watch_for_shiny_encounter(env, context);
            if (ret < 0 ){
                continue;
                // keep going if "Not even a nibble..."
            }
            shiny_found = ret == 1;
            break;
        case Target::safarizonecenter:
            if (INGAME_DELAY < 21000){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Safari Zone Center: in-game delay cannot be less than 21000ms (2520 frames). Check your in-game advances and calibration.",
                    env.console
                );
            }
            MODIFIED_INGAME_DELAY = INGAME_DELAY - 20670;
            walk_to_safarizonecenter(context);
            use_sweet_scent(env, context, MODIFIED_INGAME_DELAY, true);
            shiny_found = watch_for_shiny_encounter(env, context) == 1;
            break;
        case Target::safarizoneeast:
            if (INGAME_DELAY < 36500){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Safari Zone East: in-game delay cannot be less than 36500ms (4380 frames). Check your in-game advances and calibration.",
                    env.console
                );
            }
            MODIFIED_INGAME_DELAY = INGAME_DELAY - 36160;
            walk_to_safarizoneeast(context);
            use_sweet_scent(env, context, MODIFIED_INGAME_DELAY, true);
            shiny_found = watch_for_shiny_encounter(env, context) == 1;
            break;
        case Target::safarizonenorth:
            if (INGAME_DELAY < 38000){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Safari Zone North: in-game delay cannot be less than 38000ms (4560 frames). Check your in-game advances and calibration.",
                    env.console
                );
            }
            MODIFIED_INGAME_DELAY = INGAME_DELAY - 37410;
            walk_to_safarizonenorth(context);
            use_sweet_scent(env, context, MODIFIED_INGAME_DELAY, true);
            shiny_found = watch_for_shiny_encounter(env, context) == 1;
            break;
        case Target::safarizonewest:
            if (INGAME_DELAY < 52000){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Safari Zone West: in-game delay cannot be less than 52000ms (6240 frames). Check your in-game advances and calibration.",
                    env.console
                );
            }
            MODIFIED_INGAME_DELAY = INGAME_DELAY - 51430;
            walk_to_safarizonewest(context);
            use_sweet_scent(env, context, MODIFIED_INGAME_DELAY, true);
            shiny_found = watch_for_shiny_encounter(env, context) == 1;
            break;
        case Target::safarizonesurf:
            if (INGAME_DELAY < 31000){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Safari Zone Surfing: in-game delay cannot be less than 31000ms (3720 frames). Check your in-game advances and calibration.",
                    env.console
                );
            }
            MODIFIED_INGAME_DELAY = INGAME_DELAY - 30300;
            walk_to_safarizonesurf(context);
            use_sweet_scent(env, context, MODIFIED_INGAME_DELAY, true);
            shiny_found = watch_for_shiny_encounter(env, context) == 1;
            break;
        case Target::safarizonefish:
            if (INGAME_DELAY < 24500){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Safari Zone Fishing: in-game delay cannot be less than 24500ms (2940 frames). Check your in-game advances and calibration.",
                    env.console
                );
            }
            MODIFIED_INGAME_DELAY = INGAME_DELAY - 30300;
            walk_to_safarizonefish(context);
            use_registered_fishing_rod(env, context, MODIFIED_INGAME_DELAY);
            ret = watch_for_shiny_encounter(env, context);
            if (ret < 0 ){
                continue;
                // keep going if "Not even a nibble..."
            }
            shiny_found = ret == 1;
            break;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Option not yet implemented.",
                env.console
            );
        }

        stats.resets++;
        if (shiny_found){
            env.log("Shiny found!");
            stats.shinies++;
            send_program_notification(
                env,
                NOTIFICATION_SHINY,
                COLOR_YELLOW,
                "Shiny found!",
                {}, "",
                env.console.video().snapshot(),
                true
            );
            if (TAKE_VIDEO){
                pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 0ms);
            }
            break;
        }else if (stats.resets >= NUM_RESETS){
            send_program_status_notification(
                env, NOTIFICATION_STATUS_UPDATE,
                "Maximum resets reached."
            );
            break;
        }else{
            env.log("Pokemon is not shiny.");
            env.log("Resetting.");
            send_program_status_notification(
                env, NOTIFICATION_STATUS_UPDATE,
                "Resetting."
            );
            env.update_stats();
            context.wait_for_all_requests();
        }
    }

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}

