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
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Inference/NintendoSwitch_HomeMenuDetector.h"
#include "NintendoSwitch/Inference/NintendoSwitch_UpdatePopupDetector.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
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
        ProgramControllerClass::StandardController_RequiresPrecision,
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
    , SEED_BUTTON(
        "<b>Seed Button:</b><br>",
        {
            {SeedButton::A, "A", "A"},
            {SeedButton::Start, "Start", "Start"},
            {SeedButton::L, "L", "L (L=A)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        SeedButton::A
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
    , CONTINUE_SCREEN_ADVANCES(
        "<b>Continue Screen Advances (frames):</b><br>The number of frames to advance before loading the game.<br>These pass at the \"normal\" rate compared to other consoles.",
        LockMode::LOCK_WHILE_RUNNING,
        1000, 192 // default, min
    )
    , CONTINUE_SCREEN_CALIBRATION(
        "<b>Continue Screen Advances Calibration (frames):</b><br>A \"fine adjustment\" that modifies the frame advances passed on the Continue Screen.<br>"
        "Example: if your target advance was 10000 and you hit 10025, you can decrease your calibration value by 25.",
        LockMode::UNLOCK_WHILE_RUNNING,
        0 // default
    )
    , INGAME_ADVANCES(
        "<b>In-Game Advances (frames):</b><br>The number of frames to advance before triggering the gift/encounter.<br>These pass at double the rate compared to other consoles, where every 2nd frame is skipped.<br><i>Warning: this needs to be long enough to accomodate all in-game button presses prior to the gift/encounter</i>",
        LockMode::LOCK_WHILE_RUNNING,
        12345, 480 // default, min
    )
    , INGAME_CALIBRATION(
        "<b>In-Game Advances Calibration (frames):</b><br>A \"coarse adjustment\" that modifies the frame advances passed after loading the game.<br>"
        "Example: if your target advance was 10000 and you hit 8500, you can increase your calibration value by 1500.",
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
    PA_ADD_OPTION(SEED_BUTTON);
    PA_ADD_OPTION(SEED_DELAY);
    PA_ADD_OPTION(SEED_CALIBRATION);
    PA_ADD_OPTION(CONTINUE_SCREEN_ADVANCES);
    PA_ADD_OPTION(CONTINUE_SCREEN_CALIBRATION);
    PA_ADD_OPTION(INGAME_ADVANCES);
    PA_ADD_OPTION(INGAME_CALIBRATION);
    PA_ADD_OPTION(USE_COPYRIGHT_TEXT);
    PA_ADD_OPTION(USE_TEACHY_TV);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

namespace{

void collect_starter_after_delay(ProControllerContext& context, const uint64_t& INGAME_DELAY){
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

void collect_magikarp_after_delay(ProControllerContext& context, const uint64_t& INGAME_DELAY){
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

void collect_hitmon_after_delay(ProControllerContext& context, const uint64_t& INGAME_DELAY){
    // One dialog before accepting
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(INGAME_DELAY - 4200)); // 4000ms + 200ms
    // Confirm selection
    pbf_press_button(context, BUTTON_A, 200ms, 1800ms);
    // Decline nickname
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
}

void collect_eevee_after_delay(ProControllerContext& context, const uint64_t& INGAME_DELAY){
    // No dialogue to advance through -- just wait
    pbf_wait(context, std::chrono::milliseconds(INGAME_DELAY - 4000));
    // Interact with the pokeball
    pbf_press_button(context, BUTTON_A, 200ms, 3800ms);
    // Decline nickname
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
}

void collect_lapras_after_delay(ProControllerContext& context, const uint64_t& INGAME_DELAY){
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

void collect_fossil_after_delay(ProControllerContext& context, const uint64_t& INGAME_DELAY){
    // 2 dialog presses
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(INGAME_DELAY - 5700)); // 4000ms + 1500ms + 200ms
    // Advance dialog on target frame
    pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
    // Decline nickname
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
}

void collect_gamecorner_after_delay(ProControllerContext& context, const uint64_t& INGAME_DELAY, int SLOT){
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

void collect_togepi_egg_after_delay(ProControllerContext& context, const uint64_t& INGAME_DELAY){
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

void encounter_static_after_delay(ProControllerContext& context, const uint64_t& INGAME_DELAY){
    // No dialogue to advance through -- just wait in the start menu (avoids extra RNG advances by boulders Mt Ember and Seafoam Islands)
    pbf_press_button(context, BUTTON_PLUS, 200ms, 300ms);
    pbf_wait(context, std::chrono::milliseconds(INGAME_DELAY - 5000)); // 4000ms + 1000ms
    pbf_press_button(context, BUTTON_B, 200ms, 300ms);
    // Interact with the static encounter
    pbf_press_button(context, BUTTON_A, 200ms, 800ms);
    pbf_mash_button(context, BUTTON_A, 1000ms); // finishes dialog for the legendary birds
    context.wait_for_all_requests();
}

void encounter_snorlax_after_delay(ProControllerContext& context, const uint64_t& INGAME_DELAY){
    // Interact with Snorlax, YES to PokeFlute, wait on "woke up!"
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 9800ms); // PokeFlute tune
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(INGAME_DELAY - 15700)); // 4000ms + 1500ms + 10000ms + 200ms
    pbf_press_button(context, BUTTON_A, 200ms, 200ms); 
    context.wait_for_all_requests();
}

void encounter_mewtwo_after_delay(ProControllerContext& context, const uint64_t& INGAME_DELAY){
    // one dialogue before the encounter happens
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(INGAME_DELAY - 4200)); // 4000ms + 200ms
    // Initiate encounter
    pbf_press_button(context, BUTTON_A, 200ms, 200ms); 
    context.wait_for_all_requests();
}

void encounter_hooh_after_delay(ProControllerContext& context, const uint64_t& INGAME_DELAY){
    // No dialogue to advance through -- just wait
    pbf_wait(context, std::chrono::milliseconds(INGAME_DELAY - 4000));
    // Trigger the encounter (WALK UP)
    pbf_move_left_joystick(context, {0, +1}, 800ms, 700ms);
    context.wait_for_all_requests();
}

void encounter_hypno_after_delay(ProControllerContext& context, const uint64_t& INGAME_DELAY){
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

void use_sweet_scent(ProControllerContext& context, const uint64_t& INGAME_DELAY, bool SAFARI_ZONE = false){
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

void use_registered_fishing_rod(ProControllerContext& context, const uint64_t& INGAME_DELAY){
    uint32_t rng_wait = 50 * random_u32(0, 20); // helps avoid always hitting "Not even a nibble" (?)
    pbf_wait(context, std::chrono::milliseconds(rng_wait));
    pbf_press_button(context, BUTTON_MINUS, 200ms, std::chrono::milliseconds(INGAME_DELAY - rng_wait - 4200));
    pbf_press_button(context, BUTTON_A, 200ms, 800ms);
    context.wait_for_all_requests();
}

void go_to_starter_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // Navigate to summary (1st party slot)
    open_party_menu_from_overworld(env.console, context);
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
    open_party_menu_from_overworld(env.console, context);
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


void RngHelper::set_seed_after_delay(ProControllerContext& context, int64_t& FIXED_SEED_OFFSET){
    // wait on title screen for the specified delay
    pbf_wait(context, std::chrono::milliseconds(SEED_DELAY + SEED_CALIBRATION + FIXED_SEED_OFFSET));
    // hold the specified button for a few seconds through the transition to the Continue Screen
    Button button;
    switch (SEED_BUTTON){
    case SeedButton::A:
        button = BUTTON_A;
        break;
    case SeedButton::Start:
        button = BUTTON_PLUS;
        break;
    case SeedButton::L:
        button = BUTTON_L;
        break;
    default:
        button = BUTTON_A;
        break;
    }
    BUTTON_A;
    pbf_press_button(context, button, 3000ms, 0ms);
}

void RngHelper::load_game_after_delay(ProControllerContext& context, const uint64_t& CONTINUE_SCREEN_DELAY){
    pbf_wait(context, std::chrono::milliseconds(CONTINUE_SCREEN_DELAY - 3000));
    pbf_press_button(context, BUTTON_A, 33ms, 1467ms);
    // skip recap
    pbf_press_button(context, BUTTON_B, 33ms, 2467ms);
    // need to later subtract 4000ms from delay to hit desired number of advances
}

void RngHelper::wait_with_teachy_tv(ProControllerContext& context, const uint64_t& TEACHY_DELAY){
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

void RngHelper::check_timings(SingleSwitchProgramEnvironment& env, int64_t FIXED_SEED_OFFSET, const uint64_t& CONTINUE_SCREEN_DELAY, const uint64_t& INGAME_DELAY, bool SAFARI_ZONE){
    if (CONTINUE_SCREEN_DELAY < 3200){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "The Continue Screen delay cannot be less than 3200ms (192 frames). Check your Continue Screen calibration.",
            env.console
        );
    }
    if (SEED_DELAY + SEED_CALIBRATION + FIXED_SEED_OFFSET < 28000){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "The title screen delay cannot be less than 28000ms. Check your seed calibration.",
            env.console
        );
    }
    
    switch (TARGET){
    case Target::starters:
        if (INGAME_DELAY < 7500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Starters: the in-game delay cannot be less than 7500ms (900 frames). Check your in-game advances and calibration.",
                env.console
            );
        }
        return;
    case Target::magikarp:
        if (INGAME_DELAY < 7500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Magikarp: the in-game delay cannot be less than 7500ms (900 frames). Check your in-game advances and calibration.",
                env.console
            );
        }
        return;
    case Target::hitmon:
        if (INGAME_DELAY < 4500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Hitmonchan/Hitmonlee: the in-game delay cannot be less than 4500ms (540 frames). Check your in-game advances and calibration.",
                env.console
            );
        }
        return;
    case Target::eevee:
        if (INGAME_DELAY < 4000){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Eevee: the in-game delay cannot be less than 4000ms (480 frames). Check your in-game advances and calibration.",
                env.console
            );
        }
        return;
    case Target::lapras:
        if (INGAME_DELAY < 7500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Lapras: the in-game delay cannot be less than 7500ms (900 frames). Check your in-game advances and calibration.",
                env.console
            );
        }
        return;
    case Target::fossils:
        if (INGAME_DELAY < 6000){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Fossils: the in-game delay cannot be less than 6000ms (720 frames). Check your in-game advances and calibration.",
                env.console
            );
        }
        return;
    case Target::gamecornerabra:
    case Target::gamecornerclefairy:
    case Target::gamecornerdratini:
    case Target::gamecornerbug:
    case Target::gamecornerporygon:
        if (INGAME_DELAY < 8500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Game Corner: the in-game delay cannot be less than 8500ms (1020 frames). Check your in-game advances and calibration.",
                env.console
            );
        }
        return;
    case Target::togepi:
        if (INGAME_DELAY < 12000) {
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Togepi: the in-game delay cannot be less than 12000ms (1440 frames). Check your in-game advances and calibration.",
                env.console
            );
        }
        return;
    case Target::staticencounter:
        if (INGAME_DELAY < 5000){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Static Encounter: the in-game delay cannot be less than 5000ms (600 frames). Check your in-game advances and calibration.",
                env.console
            );
        }
        return;
    case Target::snorlax:
        if (INGAME_DELAY < 16000){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Snorlax: the in-game delay cannot be less than 16000ms (1920 frames). Check your in-game advances and calibration.",
                env.console
            );
        }
        return;
    case Target::mewtwo:
        if (INGAME_DELAY < 4500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Mewtwo: the in-game delay cannot be less than 4500ms (540 frames). Check your in-game advances and calibration.",
                env.console
            );
        }
        return;
    case Target::hooh:
        if (INGAME_DELAY < 4000){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Ho-oh: the in-game delay cannot be less than 4000ms (480 frames). Check your in-game advances and calibration.",
                env.console
            );
        }
        return;
    case Target::hypno:
        if (INGAME_DELAY < 13000){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Hypno: the in-game delay cannot be less than 13000ms (1560 frames). Check your in-game advances and calibration.",
                env.console
            );
        }
        return;
    case Target::sweetscent:
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
        return;
    case Target::fishing:
        if (INGAME_DELAY < 5500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Fishing: the in-game delay cannot be less than 5500ms (1800 frames). Check your in-game advances and calibration.",
                env.console
            );
        }
        return;
    case Target::safarizonecenter:
        if (INGAME_DELAY < 30500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Safari Zone Center: in-game delay cannot be less than 30500ms (3660 frames). Check your in-game advances and calibration.",
                env.console
            );
        }
        return;
    case Target::safarizoneeast:
        if (INGAME_DELAY < 36500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Safari Zone East: in-game delay cannot be less than 36500ms (4380 frames). Check your in-game advances and calibration.",
                env.console
            );
        }
        return;
    case Target::safarizonenorth:
        if (INGAME_DELAY < 47500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Safari Zone North: in-game delay cannot be less than 47500ms (5700 frames). Check your in-game advances and calibration.",
                env.console
            );
        }
        return;
    case Target::safarizonewest:
        if (INGAME_DELAY < 61500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Safari Zone West: in-game delay cannot be less than 52000ms (7380 frames). Check your in-game advances and calibration.",
                env.console
            );
        }
        return;
    case Target::safarizonesurf:
        if (INGAME_DELAY < 40500){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Safari Zone Surfing: in-game delay cannot be less than 40500ms (4860 frames). Check your in-game advances and calibration.",
                env.console
            );
        }
        return;
    case Target::safarizonefish:
        if (INGAME_DELAY < 30000){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Safari Zone Fishing: in-game delay cannot be less than 30000ms (3600 frames). Check your in-game advances and calibration.",
                env.console
            );
        }
        return;
    default:
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Option not yet implemented.",
            env.console
        );
    }
}

void RngHelper::perform_blind_sequence(ProControllerContext& context, int64_t FIXED_SEED_OFFSET, const uint64_t& CONTINUE_SCREEN_DELAY, const uint64_t& TEACHY_DELAY, const uint64_t& INGAME_DELAY, bool SAFARI_ZONE){
    pbf_press_button(context, BUTTON_A, 80ms, 0ms); // start the game from the Home screen
    set_seed_after_delay(context, FIXED_SEED_OFFSET);
    load_game_after_delay(context, CONTINUE_SCREEN_DELAY);
    if (TEACHY_DELAY > 0){
        wait_with_teachy_tv(context, TEACHY_DELAY);
    }

    uint64_t MODIFIED_INGAME_DELAY;
    switch (TARGET){
    case Target::starters:
        collect_starter_after_delay(context, INGAME_DELAY);
        return;
    case Target::magikarp:
        collect_magikarp_after_delay(context, INGAME_DELAY);
        return;
    case Target::hitmon:
        collect_hitmon_after_delay(context, INGAME_DELAY);
        return;
    case Target::eevee:
        collect_eevee_after_delay(context, INGAME_DELAY);
        return;
    case Target::lapras:
        collect_lapras_after_delay(context, INGAME_DELAY);
        return;
    case Target::fossils:
        collect_fossil_after_delay(context, INGAME_DELAY);
        return;
    case Target::gamecornerabra:
        collect_gamecorner_after_delay(context, INGAME_DELAY, 0);
        return;
    case Target::gamecornerclefairy:
        collect_gamecorner_after_delay(context, INGAME_DELAY, 1);
        return;
    case Target::gamecornerdratini:
        collect_gamecorner_after_delay(context, INGAME_DELAY, 2);
        return;
    case Target::gamecornerbug:
        collect_gamecorner_after_delay(context, INGAME_DELAY, 3);
        return;
    case Target::gamecornerporygon:
        collect_gamecorner_after_delay(context, INGAME_DELAY, 4);
        return;
    case Target::togepi:
        collect_togepi_egg_after_delay(context, INGAME_DELAY);
        return;
    case Target::staticencounter:
        encounter_static_after_delay(context, INGAME_DELAY);
        return;
    case Target::snorlax:
        encounter_snorlax_after_delay(context, INGAME_DELAY);
        return;
    case Target::mewtwo:
        encounter_mewtwo_after_delay(context, INGAME_DELAY);
        return;
    case Target::hooh:
        encounter_hooh_after_delay(context, INGAME_DELAY);
        return;
    case Target::hypno:
        encounter_hypno_after_delay(context, INGAME_DELAY);
        return;
    case Target::sweetscent:
        use_sweet_scent(context, INGAME_DELAY, SAFARI_ZONE);
        return;
    case Target::fishing:
        use_registered_fishing_rod(context, INGAME_DELAY);
        return;
    case Target::safarizonecenter:
        MODIFIED_INGAME_DELAY = INGAME_DELAY - 20670;
        walk_to_safarizonecenter(context);
        use_sweet_scent(context, MODIFIED_INGAME_DELAY, true);
        return;
    case Target::safarizoneeast:
        MODIFIED_INGAME_DELAY = INGAME_DELAY - 36160;
        walk_to_safarizoneeast(context);
        use_sweet_scent(context, MODIFIED_INGAME_DELAY, true);
        return;
    case Target::safarizonenorth:
        MODIFIED_INGAME_DELAY = INGAME_DELAY - 37410;
        walk_to_safarizonenorth(context);
        use_sweet_scent(context, MODIFIED_INGAME_DELAY, true);
        return;
    case Target::safarizonewest:
        MODIFIED_INGAME_DELAY = INGAME_DELAY - 51430;
        walk_to_safarizonewest(context);
        use_sweet_scent(context, MODIFIED_INGAME_DELAY, true);
    case Target::safarizonesurf:
        MODIFIED_INGAME_DELAY = INGAME_DELAY - 30300;
        walk_to_safarizonesurf(context);
        use_sweet_scent(context, MODIFIED_INGAME_DELAY, true);
        return;
    case Target::safarizonefish:
        MODIFIED_INGAME_DELAY = INGAME_DELAY - 30300;
        walk_to_safarizonefish(context);
        use_registered_fishing_rod(context, MODIFIED_INGAME_DELAY);
        return;
    }
}

void RngHelper::reset_and_perform_blind_sequence(SingleSwitchProgramEnvironment& env, ProControllerContext& context, int64_t FIXED_SEED_OFFSET, const uint64_t& CONTINUE_SCREEN_DELAY, const uint64_t& TEACHY_DELAY, const uint64_t& INGAME_DELAY, bool SAFARI_ZONE){
    go_home(env.console, context);
    close_game_from_home(env.console, context);
    start_game_from_home(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_SLOW, uint8_t(0), uint8_t(0)); // TODO: add option for user slot if needed
    go_home(env.console, context); // happens as soon as a black screen is detected

    // attempt to resume the game and perform the blind sequence
    // by this point, the license check should be over, so we don't need to worry about it when resuming the game
    // profile selection is also already taken care of
    uint8_t attempts = 0;
    while(true){
        if (attempts >= 5){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "RngHelper(): Failed to resume the game 5 times in a row.",
                env.console
            );  
        }

        UpdateMenuWatcher update_detector(env.console);
        context.wait_for_all_requests();
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [this, FIXED_SEED_OFFSET, CONTINUE_SCREEN_DELAY, TEACHY_DELAY, INGAME_DELAY, SAFARI_ZONE](ProControllerContext& context) {
                pbf_press_button(context, BUTTON_A, 80ms, 0ms);
                perform_blind_sequence(context, FIXED_SEED_OFFSET, CONTINUE_SCREEN_DELAY, TEACHY_DELAY, INGAME_DELAY, SAFARI_ZONE);
            },
            { update_detector }
        );

        switch (ret){
        case 0: 
            attempts++;
            env.log("Detected update window.", COLOR_RED);
            pbf_press_dpad(context, DPAD_UP, 40ms, 0ms);
            pbf_press_button(context, BUTTON_A, 80ms, 4000ms);
            context.wait_for_all_requests();
            continue;
        default:
            return;
        }
    }
}

void RngHelper::reset_and_detect_copyright_text(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    go_home(env.console, context);
    close_game_from_home(env.console, context);
    start_game_from_home(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_SLOW, uint8_t(0), uint8_t(0)); // TODO: add option for user slot if needed
    go_home(env.console, context); // happens as soon as a black screen is detected

    uint8_t attempts = 0;
    while(true){
        if (attempts >= 5){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "RngHelper(): Failed to resume the game 5 times in a row.",
                env.console
            );  
        }

        UpdateMenuWatcher update_detector(env.console);
        BlackScreenWatcher blackscreen_detector(COLOR_RED);
        context.wait_for_all_requests();
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context) {
                pbf_press_button(context, BUTTON_A, 80ms, 9920ms);
            },
            { update_detector, blackscreen_detector },
            1ms
        );

        BlackScreenOverWatcher copyright_detector(COLOR_RED);
        int ret2;
        switch (ret){
        case 0: 
            attempts++;
            env.log("Detected update window.", COLOR_RED);
            pbf_press_dpad(context, DPAD_UP, 40ms, 0ms);
            pbf_press_button(context, BUTTON_A, 80ms, 4000ms);
            context.wait_for_all_requests();
            continue;
        case 1: 
            context.wait_for_all_requests();
            ret2 = wait_until(
                env.console, context, 10000ms,
                {copyright_detector },
                1ms
            );
            if (ret2 < 0){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Black screen detected for more than 10 seconds after starting game.",
                    env.console
                );
            }
            return;
        default:
            env.log("No black screen or update popup detected. Pressing A again...");
            continue;
        }
    }

}

bool RngHelper::check_for_shiny(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    switch (TARGET){
    case Target::starters:
        return shiny_check_starter_summary(env, context);
    case Target::togepi:
        hatch_togepi_egg(env, context);
    case Target::magikarp:
    case Target::hitmon:
    case Target::eevee:
    case Target::lapras:
    case Target::fossils:
    case Target::gamecornerabra:
    case Target::gamecornerclefairy:
    case Target::gamecornerdratini:
    case Target::gamecornerbug:
    case Target::gamecornerporygon:
        return shiny_check_summary(env, context);
    case Target::staticencounter:
    case Target::snorlax:
    case Target::mewtwo:
    case Target::hooh:
    case Target::hypno:
    case Target::sweetscent:
    case Target::fishing:
    case Target::safarizonecenter:
    case Target::safarizoneeast:
    case Target::safarizonenorth:
    case Target::safarizonewest:
    case Target::safarizonesurf:
    case Target::safarizonefish:
        return watch_for_shiny_encounter(env, context) == 1;
    default:
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Option not yet implemented.",
            env.console
        );
    }
}


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
        // prepare timings
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

        bool should_use_teachy_tv = USE_TEACHY_TV && (TARGET != Target::starters) && (MODIFIED_INGAME_ADVANCES > TEACHY_TV_BUFFER); // don't use Teachy TV for short in-game advance targets
        if (should_use_teachy_tv) {
            TEACHY_ADVANCES = uint64_t((int)std::floor((MODIFIED_INGAME_ADVANCES - TEACHY_TV_BUFFER) / 313) * 313);
        }

        const uint64_t CONTINUE_SCREEN_DELAY = uint64_t((CONTINUE_SCREEN_ADVANCES + CONTINUE_SCREEN_CALIBRATION) * FRAME_DURATION);
        const uint64_t TEACHY_DELAY = uint64_t(TEACHY_ADVANCES * FRAME_DURATION / 313);
        const uint64_t INGAME_DELAY = uint64_t((MODIFIED_INGAME_ADVANCES - TEACHY_ADVANCES) * FRAME_DURATION / 2) - (should_use_teachy_tv ? 13700 : 0);
        env.log("Continue Screen delay: " + std::to_string(CONTINUE_SCREEN_DELAY) + "ms");
        env.log("In-game delay: " + std::to_string(INGAME_DELAY) + "ms");
        env.log("Teachy TV delay: " + std::to_string(TEACHY_DELAY) + "ms");
        env.log("Total time: " + std::to_string(SEED_DELAY + SEED_CALIBRATION + FIXED_SEED_OFFSET + CONTINUE_SCREEN_DELAY + INGAME_DELAY + TEACHY_DELAY) + "ms");


        // handle the blind part
        if (USE_COPYRIGHT_TEXT){
            reset_and_detect_copyright_text(env, context);
            perform_blind_sequence(context, FIXED_SEED_OFFSET, CONTINUE_SCREEN_DELAY, TEACHY_DELAY, INGAME_DELAY, SAFARI_ZONE);
        }else{
            reset_and_perform_blind_sequence(env, context, FIXED_SEED_OFFSET, CONTINUE_SCREEN_DELAY, TEACHY_DELAY, INGAME_DELAY, SAFARI_ZONE);
        }
        stats.resets++;

        // detect shinies
        shiny_found = check_for_shiny(env, context);
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

