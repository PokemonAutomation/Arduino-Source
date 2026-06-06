/*  Egg RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <algorithm>
#include "CommonTools/Random.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Language.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_DaycareManDetector.h"
#include "PokemonFRLG/Programs/PokemonFRLG_StartMenuNavigation.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_RngNavigation.h"
#include "PokemonFRLG_HardReset.h"
#include "PokemonFRLG_RngCalibration.h"
#include "PokemonFRLG_EncountersDatabase.h"
#include "PokemonFRLG_EggRng.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

ParentIVsRow::ParentIVsRow(int index, std::string&& label)
    : StaticTableRow(label)
    , parent(LockMode::LOCK_WHILE_RUNNING, label)
    , hp(LockMode::LOCK_WHILE_RUNNING, 31, 0, 31)
    , atk(LockMode::LOCK_WHILE_RUNNING, 31, 0, 31)
    , def(LockMode::LOCK_WHILE_RUNNING, 31, 0, 31)
    , spa(LockMode::LOCK_WHILE_RUNNING, 31, 0, 31)
    , spd(LockMode::LOCK_WHILE_RUNNING, 31, 0, 31)
    , spe(LockMode::LOCK_WHILE_RUNNING, 31, 0, 31)
    , index(index)
{
    PA_ADD_OPTION(parent);
    PA_ADD_OPTION(hp);
    PA_ADD_OPTION(atk);
    PA_ADD_OPTION(def);
    PA_ADD_OPTION(spa);
    PA_ADD_OPTION(spd);
    PA_ADD_OPTION(spe);
}

ParentIVsTable::ParentIVsTable()
    : StaticTableOption("<b>Parent IVs</b>.", LockMode::LOCK_WHILE_RUNNING)
{
    add_row(std::make_unique<ParentIVsRow>(0, "Parent A"));
    add_row(std::make_unique<ParentIVsRow>(1, "Parent B"));
    finish_construction();
}
std::vector<std::string> ParentIVsTable::make_header() const{
    std::vector<std::string> ret{
        "Parent",
        "HP",
        "Attack",
        "Defense",
        "Sp.Attack",
        "Sp.Defense",
        "Speed"
    };
    return ret;
}


EggRng_Descriptor::EggRng_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:EggRng",
        Pokemon::STRING_POKEMON + " FRLG", "Egg RNG",
        "Programs/PokemonFRLG/EggRng.html",
        "Automatically calibrate timings to hit a specific RNG target for daycare eggs.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct EggRng_Descriptor::Stats : public StatsTracker{
    Stats()
        : resets(m_stats["Resets"])
        , shinies(m_stats["Shinies"])
        , nonshiny(m_stats["Non-Shiny Hits"])
        , balls(m_stats["Balls Used"])
        , candies(m_stats["Rare Candies Used"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Non-Shiny Hits", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Balls Used", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Rare Candies Used", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& nonshiny;
    std::atomic<uint64_t>& balls;
    std::atomic<uint64_t>& candies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> EggRng_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

EggRng::EggRng()
    : m_calibration_displays("Calibration Displays")
    , HELD_CALIBRATION("Held Frame Calibration")
    , PICKUP_CALIBRATION("Pickup Frame Calibration")
    , m_game_info("Game Information")
    , LANGUAGE(
        "<b>Game Language:</b>",
        {
            Language::English,
            Language::Japanese,
            Language::Spanish,
            Language::French,
            Language::German,
            Language::Italian,
        },
        LockMode::LOCK_WHILE_RUNNING,
        true
        )
    , GAME_VERSION(
        "<b>Game Version:</b>",
        {
            {GameVersion::firered, "firered", "FireRed"},
            {GameVersion::leafgreen, "leafgreen", "LeafGreen"}
        },
        LockMode::LOCK_WHILE_RUNNING,
        GameVersion::firered
        )
    , m_target_settings("Target Settings")
    , EGG_SPECIES(
        "<b>Egg Species:</b>",
        "PokemonFRLG/EggSpeciesGen3.json"
        )
    , COMPATIBILITY(
        "<b>Parent Compatiblity:</b>",
        {
            {AdvEggCompatibility::low, "low", "The two don't seem to like each other"},
            {AdvEggCompatibility::medium, "medium", "The two seem to get along"},
            {AdvEggCompatibility::high, "high", "The two seem to get along very well"}
        },
        LockMode::LOCK_WHILE_RUNNING,
        AdvEggCompatibility::low
        )
    , m_held_settings("Held Frame Settings")
    , HELD_SEED(
        false,
        "<b>Held Seed:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "70FE", "70FE",
        true
        )
    , HELD_SEED_LIST(
        "<b>Nearby Held Seeds:</b><br>"
        "This box should contain a list of seeds (in order) around and including your held seed, with one seed on each line",
        LockMode::LOCK_WHILE_RUNNING,
        "D000\n199A\n77A1\nAABC\n280C\n70FE\nB573\n02F2\n8084\nA533\nED1E",
        "D000\n199A\n77A1\nAABC\n280C\n70FE\nB573\n02F2\n8084\nA533\nED1E",
        true
        )
    , HELD_SEED_BUTTON(
        "<b>Held Seed Button:</b><br>",
        {
            {SeedButton::A, "A", "A"},
            {SeedButton::Start, "Start", "Start"},
            {SeedButton::L, "L", "L (L=A)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        SeedButton::A
        )
    , HELD_EXTRA_BUTTON(
        "<b>Held Seed Extra Button:</b><br>"
        "Additional button presses that affect the seed.",
        {
            {BlackoutButton::None, "None", "None"},
            {BlackoutButton::L, "L", "Blackout L"},
            {BlackoutButton::R, "R", "Blackout R"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        BlackoutButton::None
        )
    , HELD_SEED_DELAY(
        "<b>Held Seed Delay Time (ms):</b><br>"
        "The delay between starting the game and advancing past the title screen. Set this to match your held seed.<br>"
        "<i>If using Ten Lines, select <b>Nintendo Switch 1</b> as your console even if using a Switch 2.</i>",
        LockMode::LOCK_WHILE_RUNNING,
        31338, 30400 // default, min
        )
    , HELD_ADVANCES(
        "<b>Held Advances:</b><br>The total number of RNG advances for your held target.",
        LockMode::LOCK_WHILE_RUNNING,
        10000, 700, 1000000000 // default, min
        )
    , m_pickup_settings("Pickup Frame Settings")
    , PICKUP_SEED(
        false,
        "<b>Pickup Seed:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "70FE", "70FE",
        true
        )
    , PICKUP_SEED_LIST(
        "<b>Nearby Pickup Seeds:</b><br>"
        "This box should contain a list of seeds (in order) around and including your held seed, with one seed on each line",
        LockMode::LOCK_WHILE_RUNNING,
        "D000\n199A\n77A1\nAABC\n280C\n70FE\nB573\n02F2\n8084\nA533\nED1E",
        "D000\n199A\n77A1\nAABC\n280C\n70FE\nB573\n02F2\n8084\nA533\nED1E",
        true
        )
    , PICKUP_SEED_BUTTON(
        "<b>Pickup Seed Button:</b><br>",
        {
            {SeedButton::A, "A", "A"},
            {SeedButton::Start, "Start", "Start"},
            {SeedButton::L, "L", "L (L=A)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        SeedButton::A
        )
    , PICKUP_EXTRA_BUTTON(
        "<b>Pickup Seed Extra Button:</b><br>"
        "Additional button presses that affect the seed.",
        {
            {BlackoutButton::None, "None", "None"},
            {BlackoutButton::L, "L", "Blackout L"},
            {BlackoutButton::R, "R", "Blackout R"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        BlackoutButton::None
        )
    , PICKUP_SEED_DELAY(
        "<b>Pickup Seed Delay Time (ms):</b><br>"
        "The delay between starting the game and advancing past the title screen. Set this to match your held seed.<br>"
        "<i>If using Ten Lines, select <b>Nintendo Switch 1</b> as your console even if using a Switch 2.</i>",
        LockMode::LOCK_WHILE_RUNNING,
        31338, 30400 // default, min
        )
    , PICKUP_ADVANCES(
        "<b>Pickup Advances:</b><br>The total number of RNG advances for your pickup target.",
        LockMode::LOCK_WHILE_RUNNING,
        10000, 700, 1000000000 // default, min
        )
    , m_program_settings("Program Settings")
    , STARTING_POINT(
        "<b>Starting Point:</b>",
        {
            {EggProgramState::held_prep, "held_prep", "Parents Dropped Off"},
            {EggProgramState::held_calibration, "held_calibration", "Held Frame Calibration"},
            {EggProgramState::pickup_calibration, "pickup_frame", "Pickup Frame Calibration"}
        },
        LockMode::LOCK_WHILE_RUNNING,
        EggProgramState::held_prep
        )
    , MAX_RESETS(
        "<b>Max Resets:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        200, 0 // default, min
        )
    , MAX_RARE_CANDIES(
        "<b>Max Rare Candies:</b><br>"
        "The number of rare candies in your bag. Make sure these are at the top position of the bag.<br>"
        "Rare candies used during calibration will be restored after resetting.",
        LockMode::UNLOCK_WHILE_RUNNING,
        0, 0, 999 // default, min, max
        )
    , MAX_BALL_THROWS(
        "<b>Max Balls Thrown:</b><br>"
        "The number of " + STRING_POKEBALL + "s in your bag to attempt to throw. Make sure these are at the top position of the bag.<br>"
                                "Balls thrown during calibration will be restored after resetting.",
        LockMode::UNLOCK_WHILE_RUNNING,
        20, 2, 999 // default, min, max
        )
    , USE_TEACHY_TV(
        "<b>Use Teachy TV:</b>"
        "<br>Opens the Teachy TV to quickly advance the RNG at 313x speed.<br>"
        "<i>Warning: can result in larger misses.</i>",
        LockMode::LOCK_WHILE_RUNNING,
        false // default
        )
    , PROFILE(
        "<b>User Profile Position:</b><br>"
        "The position, from left to right, of the Switch profile with the FRLG save you'd like to use.<br>"
        "If this is set to 0, Switch 1 defaults to the last-used profile, while Switch 2 defaults to the first profile (position 1)",
        LockMode::LOCK_WHILE_RUNNING,
        0, 0, 8 // default, min, max
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
    PA_ADD_OPTION(m_calibration_displays);
    PA_ADD_OPTION(RNG_TARGET);
    PA_ADD_OPTION(RNG_FILTERS);
    PA_ADD_OPTION(HELD_CALIBRATION);
    PA_ADD_OPTION(PICKUP_CALIBRATION);
    PA_ADD_OPTION(m_game_info);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(GAME_VERSION);
    PA_ADD_OPTION(m_target_settings);
    PA_ADD_OPTION(EGG_SPECIES);
    PA_ADD_OPTION(COMPATIBILITY);
    PA_ADD_OPTION(PARENT_IVS);
    PA_ADD_OPTION(m_held_settings);
    PA_ADD_OPTION(HELD_SEED);
    PA_ADD_OPTION(HELD_SEED_LIST);
    PA_ADD_OPTION(HELD_SEED_BUTTON);
    PA_ADD_OPTION(HELD_EXTRA_BUTTON);
    PA_ADD_OPTION(HELD_SEED_DELAY);
    PA_ADD_OPTION(HELD_ADVANCES);
    PA_ADD_OPTION(m_pickup_settings);
    PA_ADD_OPTION(PICKUP_SEED);
    PA_ADD_OPTION(PICKUP_SEED_LIST);
    PA_ADD_OPTION(PICKUP_SEED_BUTTON);
    PA_ADD_OPTION(PICKUP_EXTRA_BUTTON);
    PA_ADD_OPTION(PICKUP_SEED_DELAY);
    PA_ADD_OPTION(PICKUP_ADVANCES);
    PA_ADD_OPTION(m_program_settings);
    PA_ADD_OPTION(STARTING_POINT);
    PA_ADD_OPTION(MAX_RESETS);
    PA_ADD_OPTION(MAX_BALL_THROWS);
    PA_ADD_OPTION(MAX_RARE_CANDIES);
    PA_ADD_OPTION(USE_TEACHY_TV);
    PA_ADD_OPTION(PROFILE);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);

    STARTING_POINT.add_listener(*this);
}

void EggRng::on_config_value_changed(void* object){
    // switch between separate calibrations for the held and pickup frames
    if (STARTING_POINT.current_value() == 2){
        HELD_CALIBRATION.set_visibility(ConfigOptionState::HIDDEN);
        PICKUP_CALIBRATION.set_visibility(ConfigOptionState::ENABLED);
    }else{
        HELD_CALIBRATION.set_visibility(ConfigOptionState::ENABLED);
        PICKUP_CALIBRATION.set_visibility(ConfigOptionState::HIDDEN);
    }
}


namespace {
    void use_max_repel(ConsoleHandle& console, ProControllerContext& context){
        console.log("Using a Max Repel (2nd bag slot from top)...");
        open_bag_from_overworld(console, context);
        pbf_move_left_joystick(context, {-1, 0}, 100ms, 900ms);
        pbf_move_left_joystick(context, {-1, 0}, 100ms, 900ms);
        pbf_move_left_joystick(context, {-1, 0}, 100ms, 900ms);
        for (int i=0; i<20; i++){
            pbf_move_left_joystick(context, {0, +1}, 100ms, 100ms);
        }
        pbf_move_left_joystick(context, {0, -1}, 100ms, 300ms);

        pbf_mash_button(context, BUTTON_A, 3000ms);
        pbf_mash_button(context, BUTTON_B, 5000ms);

    }

    void daycare_steps(ConsoleHandle& console, ProControllerContext& context){
        console.log("Taking 250 steps...");
        // walk down to the south wall
        pbf_move_left_joystick(context, {0, -1}, 930ms, 570ms);
        // walk to the southeast corner
        pbf_move_left_joystick(context, {+1, 0}, 2500ms, 300ms);

        WhiteDialogWatcher repel_over(COLOR_RED);
        context.wait_for_all_requests();
        int ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context) {
                pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
                for (int i=0; i<20; i++){
                    pbf_move_left_joystick(context, {-1, 0}, 2700ms, 300ms);
                    pbf_move_left_joystick(context, {+1, 0}, 2700ms, 300ms);
                }
            },
            { repel_over }
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::NO_ERROR_REPORT,
                "daycare_steps(): No Max Repel dialogue box detected.",
                console
            );
        }

        console.log("Max Repel wore off. Taking 4 more steps...");
        pbf_mash_button(context, BUTTON_B, 1000ms);

        // take 4 steps to the left    
        pbf_move_left_joystick(context, {-1, 0}, 200ms, 720ms);
        pbf_move_left_joystick(context, {-1, 0}, 30ms, 720ms);
        pbf_move_left_joystick(context, {-1, 0}, 30ms, 720ms);
        pbf_move_left_joystick(context, {-1, 0}, 30ms, 720ms);

        console.log("254 steps taken.");
    }

    bool walk_from_daycare_to_pond(ConsoleHandle& console, ProControllerContext& context){
        console.log("Walking to the daycare pond...");
        pbf_move_left_joystick(context, {0, -1}, 190ms, 300ms);
        pbf_move_left_joystick(context, {+1, 0}, 2470ms, 300ms);
        pbf_move_left_joystick(context, {0, -1}, 380ms, 300ms);

        // start surfing
        WhiteDialogWatcher surf_dialog(COLOR_RED);

        context.wait_for_all_requests();
        int ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context) {
                ssf_press_left_joystick(context, {0, -1}, 0ms, 10000ms);
                ssf_mash1_button(context, BUTTON_A, 10000ms);
            },
            { surf_dialog }
        );
        if (ret < 0){
            console.log("Failed to detect surf dialog");
            return true;
        }
        console.log("Started surfing.");
        pbf_mash_button(context, BUTTON_A, 2000ms);
        context.wait_for_all_requests();
        return false;
    }

    void walk_from_pond_to_daycare_man(ConsoleHandle& console, ProControllerContext& context){
        console.log("Walking to the daycare man...");
        pbf_move_left_joystick(context, {0, +1}, 1080ms, 300ms);
        pbf_move_left_joystick(context, {-1, 0}, 1300ms, 300ms);
        pbf_move_left_joystick(context, {0, +1}, 300ms, 300ms);
    }

    void egg_pickup(ConsoleHandle& console, ProControllerContext& context){
        console.log("Picking up egg...");
        WhiteDialogWatcher dialogue(COLOR_RED);
        context.wait_for_all_requests();
        int ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context) {
                pbf_mash_button(context, BUTTON_A, 5000ms);
            },
            { dialogue }
        );        
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "egg_pickup(): Failed to initiate dialogue.",
                console
            );
        }

        pbf_mash_button(context, BUTTON_A, 5000ms);
        pbf_mash_button(context, BUTTON_B, 2500ms);
    }

    bool walk_from_daycare_man_to_pond(ConsoleHandle& console, ProControllerContext& context){
        console.log("Walking to the daycare man...");
        pbf_move_left_joystick(context, {0, -1}, 380ms, 300ms);
        pbf_move_left_joystick(context, {+1, 0}, 1340ms, 300ms);
        pbf_move_left_joystick(context, {0, -1}, 300ms, 300ms);

        // start surfing
        WhiteDialogWatcher surf_dialog(COLOR_RED);

        context.wait_for_all_requests();
        int ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context) {
                ssf_press_left_joystick(context, {0, -1}, 0ms, 10000ms);
                ssf_mash1_button(context, BUTTON_A, 10000ms);
            },
            { surf_dialog }
        );
        if (ret < 0){
            console.log("Failed to detect surf dialog");
            return true;
        }
        console.log("Started surfing.");
        pbf_mash_button(context, BUTTON_A, 2000ms);
        context.wait_for_all_requests();
        return false;
    }

} // namespace


bool EggRng::have_hit_target(SingleSwitchProgramEnvironment& env, const uint32_t& TARGET_SEED, const uint64_t& TARGET_ADVANCES, const AdvRngState& hit){
    return (hit.seed == TARGET_SEED) && (hit.advance == TARGET_ADVANCES);
}


void EggRng::prep_held_resets(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    use_max_repel(env.console, context);
    daycare_steps(env.console, context);
    save_game_to_overworld(env.console, context);
    pbf_wait(context, 5000ms);
    context.wait_for_all_requests();
}

// void EggRng::prep_pickup_resets(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

// }

bool EggRng::reset_and_check_seed(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context,
    EggRng_Descriptor::Stats& stats,
    RngUncertainHistory& wild_uncertain_history,
    RngUncertainHistory& egg_uncertain_history,    
    RngCalibrationHistory& wild_history,
    RngCalibrationHistory& egg_history,
    AdvRngWildSearcher& wild_searcher,
    RngCalibrations& calibrations,
    WallClock& timestamp,
    uint16_t& current_seed,
    uint64_t& balls_left, 
    uint64_t& candies_left,
    uint16_t& failed_searches,
    uint16_t& times_not_held,
    bool& shiny_found,
    const bool& previously_hit_held_frame,
    Milliseconds& launch_delay,
    const uint64_t& SEED_DELAY,
    const std::vector<uint16_t>& SEED_VALUES,
    const int16_t& SEED_POSITION,
    const std::set<std::string>& SPECIES_LIST,
    const RngStatsDatabase& STATS_DATA,
    bool pickup_frame
){

    static const int64_t FIXED_SEED_OFFSET = -845; // ms, approximate
    static const int64_t FIXED_ADVANCES_OFFSET = pickup_frame ? -246 : 135;    // frames, approximate

    static const uint64_t CONTINUE_SCREEN_FRAMES = 200;

    static const double SEED_BUMPS[] = { 0, 1, -1, 2, -2 };

    const uint64_t WILD_ADVANCES_RADIUS = 4096;
    
    const uint16_t MAX_HISTORY_LENGTH = 2;

    send_program_status_notification(
        env, NOTIFICATION_STATUS_UPDATE,
        pickup_frame ? "Calibrating pickup frame" : "Calibrating held frame"
    );
    env.update_stats();

    uint64_t advances = pickup_frame ? PICKUP_ADVANCES : HELD_ADVANCES;
    PokemonFRLG_RngTarget target = pickup_frame ? PokemonFRLG_RngTarget::eggpickup : PokemonFRLG_RngTarget::eggheld;  

    if (egg_history.results.size() > 0){
        calibrations = get_calibrations(env.console, egg_history, SEED_VALUES, SEED_POSITION, advances, !pickup_frame);
    }else if (wild_history.results.size() > egg_history.results.size()){
        calibrations.seed_offset = get_seed_calibration_frames(wild_history, SEED_VALUES, SEED_POSITION);
        env.log("Updated Seed Calibration (frames): " + std::to_string(calibrations.seed_offset));
    }

    // if previous resets had uncertain advances, slightly modify the seed delay to try to hit a different target
    if (pickup_frame){
        double seed_bump = SEED_BUMPS[egg_uncertain_history.results.size() % 5];
        calibrations.seed_offset += seed_bump;
    }

    // if the egg isn't ready over several resets, 
    // or if there is repeated uncertainty about advances from previous resets,
    // bump the advances
    if (!pickup_frame && !previously_hit_held_frame){
        if (egg_history.results.empty()){
            // avoid cumulative bumps when there is no history
            if (egg_uncertain_history.calibrations.size() > 0){
                calibrations = egg_uncertain_history.calibrations.back();
            }else{
                env.log("No calibration history yet. Setting calibrations from UI...");
                calibrations = {
                    HELD_CALIBRATION.seed_calibration / FRLG_FRAME_DURATION,
                    HELD_CALIBRATION.csf_calibration,
                    HELD_CALIBRATION.advances_calibration
                };
            }
        }
        double bumpval = std::floor(times_not_held / 4) + 2 * std::floor(egg_uncertain_history.results.size());
        double advances_bump = std::pow(-1, bumpval) * std::floor((bumpval+1) / 2); // 0, -0.5, +0.5, -1, +1, -2, +2...
        double orig_csf_offset = calibrations.csf_offset;
        calibrations.csf_offset = fmod(orig_csf_offset + advances_bump, 2);
        calibrations.ingame_offset += advances_bump - (calibrations.csf_offset - orig_csf_offset);
    }

    uint64_t ingame_advances = advances - CONTINUE_SCREEN_FRAMES;

    RngTimings timings = prepare_timings(
        env.console, target,
        SEED_DELAY, CONTINUE_SCREEN_FRAMES, ingame_advances,
        USE_TEACHY_TV, calibrations,
        FIXED_SEED_OFFSET, FIXED_ADVANCES_OFFSET
    );

    // Step 1: reset and perform blind sequence
    env.log("Resetting Game...");
    reset_and_perform_blind_sequence(
        env.console, context, target, 
        HELD_SEED_BUTTON, HELD_EXTRA_BUTTON,
        timings, launch_delay, false, PROFILE
    );
    stats.resets++; 

    RNG_FILTERS.reset();
    if (egg_history.results.size() > 0){
        if (pickup_frame){
            PICKUP_CALIBRATION.set_calibrations(calibrations);
        }else{
            HELD_CALIBRATION.set_calibrations(calibrations);
        }
    }

    // grab a timestamp after the blind button presses are over
    // this is used for estimating what advances wild encounters and test pickups occur on
    timestamp = current_time(); //

    // Step 2: walk to the pond
    if (pickup_frame){
        walk_from_daycare_man_to_pond(env.console, context);
    }else{
        leave_pokecenter(env.console, context);

        DaycareManWatcher egg_ready(COLOR_RED);
        context.wait_for_all_requests();
        int ret = wait_until(
            env.console, context,
            std::chrono::milliseconds(5000),
            { egg_ready }
        );

        if (ret < 0){
            env.log("Egg not ready. Resetting...");
            times_not_held++;
            return false;
        }
        env.log("Egg ready. Checking seed...");
        times_not_held = 0;

        walk_from_daycare_to_pond(env.console, context);
    }

    // Step 3: Catch a wild encounter and use it to determine the current seed
    uint32_t rng_wait = 50 * random_u32(0, 20); // avoid hitting the same wild targets
    pbf_wait(context, std::chrono::milliseconds(rng_wait));

    use_sweet_scent_from_overworld(env.console, context, pickup_frame ? 2 : 0);

    WallDuration elapsed = current_time() - timestamp;
    auto elapsed_ms = std::chrono::duration_cast<Milliseconds>(elapsed);
    uint64_t wild_advances_estimate = uint64_t(advances + 2 * elapsed_ms.count() / FRLG_FRAME_DURATION);
    env.log("Wild advances estimate: " + std::to_string(wild_advances_estimate));    

    int ret = watch_for_shiny_encounter(env.console, context);
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "EggRng(): Failed to trigger battle",
            env.console
        ); 
    }
    shiny_found = (ret == 1);

    if (shiny_found){
        return true;
    }

    int balls_thrown = auto_catch(env.console, context, balls_left);
    if (balls_thrown < 0){
        stats.errors++;
        send_program_recoverable_error_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            "auto_catch() encountered an error."
        ); 
        return false;
    }else if(balls_thrown == 0){
        env.log("Failed catch.");
        return false;
    }

    balls_left -= balls_thrown;

    go_to_summary(env.console, context);
    AdvObservedPokemon pokemon = read_summary(env.console, context, LANGUAGE, SPECIES_LIST);
    RngStats species_stats;
    try{
        species_stats = STATS_DATA.get_throw(pokemon.species);        
    }catch (const InternalProgramError& err){
        env.log(err.message());
        env.log("Failed to load base stats.");
        return false;
    }

    AdvRngFilters filters = observation_to_filters(pokemon, species_stats.base_stats, AdvRngMethod::Any);
    RNG_FILTERS.set(filters);

    std::vector<AdvRngState> search_hits = get_wild_search_results(
        env.console, wild_searcher, filters, SEED_VALUES, 
        wild_advances_estimate, WILD_ADVANCES_RADIUS, species_stats.gender_threshold
    );
    if (pickup_frame){
        PICKUP_CALIBRATION.set_hits(search_hits);
    }else{
        HELD_CALIBRATION.set_hits(search_hits);
    }

    bool finished = update_history(
        env.console, wild_uncertain_history, wild_history, MAX_HISTORY_LENGTH, 
        calibrations, search_hits, 1
    );
    bool identical = same_seeds(search_hits);

    uint64_t candies_used = 0;
    for (uint64_t i=0; i<candies_left; i++){
        if (finished || identical){
            break;
        }

        bool failed = use_rare_candy(env.console, context, LANGUAGE, pokemon, filters, species_stats.base_stats, AdvRngMethod::Any, false, i == 0);
        if (failed){
            stats.errors++;
            send_program_recoverable_error_notification(
                env, NOTIFICATION_ERROR_RECOVERABLE,
                "Failed to use Rare Candy."
            ); 
        }
        RNG_FILTERS.set(filters);
        candies_used = i;

        search_hits = get_wild_search_results(
            env.console, wild_searcher, filters, SEED_VALUES, 
            wild_advances_estimate, WILD_ADVANCES_RADIUS, species_stats.gender_threshold
        );
        if (pickup_frame){
            PICKUP_CALIBRATION.set_hits(search_hits);
        }else{
            HELD_CALIBRATION.set_hits(search_hits);
        }

        bool force_finish = failed || (i == (MAX_RARE_CANDIES - 1));
        finished = update_history(
            env.console, wild_uncertain_history, wild_history, MAX_HISTORY_LENGTH, 
            calibrations, search_hits, 1, 2, force_finish
        );
        identical = same_seeds(search_hits);
    }
    candies_left -= candies_used;

    if (search_hits.size() == 0){
        env.log("No Wild RNG matches found. Resetting...");
        failed_searches++;
        return false;
    }else{
        env.log("Wild RNG search finished.");
        failed_searches = 0;
    }

    if (search_hits[0].seed != SEED_VALUES[SEED_POSITION]){
        env.log("Wrong seed hit.");
        if (pickup_frame || previously_hit_held_frame){
            return false;
        }
    }else{
        env.log("Hit held seed.");
    }

    current_seed = search_hits[0].seed;
    return true;
}

bool EggRng::held_frame_check(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context,
    EggRng_Descriptor::Stats& stats,
    RngUncertainHistory& egg_uncertain_history,
    RngCalibrationHistory& held_calibration_history,
    AdvRngEggSearcher& egg_searcher,
    RngCalibrations& calibrations,
    const WallClock& timestamp,
    const uint16_t& current_seed, 
    const uint64_t& candies_left,
    uint16_t& failed_searches,
    bool& shiny_found,
    bool& previously_hit_held_frame,
    const uint16_t& TARGET_HELD_SEED,
    const RngStats& EGG_STATS,
    AdvIVs& PARENT_A,
    AdvIVs& PARENT_B
){
    static const uint64_t HELD_CHECK_ADVANCES_RADIUS = 8092;    

    static const uint16_t MAX_HISTORY_LENGTH = 2;
    const uint64_t INITIAL_ADVANCES_RADIUS = USE_TEACHY_TV ? 4096 : 1024;

    uint64_t advances_radius = 
        previously_hit_held_frame ? 4 
        : get_advances_radius(env.console, held_calibration_history, INITIAL_ADVANCES_RADIUS);

    pbf_mash_button(context, BUTTON_B, 5000ms); // return to overworld
    walk_from_pond_to_daycare_man(env.console, context);

    // decide if it's a good idea to save and commit to this held frame
    bool locked_in = false;
    if (previously_hit_held_frame && (current_seed == TARGET_HELD_SEED)){
        env.log("Committing to this Held Frame and saving the game...");
        save_game_to_overworld(env.console, context);
        locked_in = true;
    }

    // collect and hatch egg
    egg_pickup(env.console, context);
    WallDuration elapsed = current_time() - timestamp;
    pbf_press_button(context, BUTTON_MINUS, 200ms, 800ms); // hop on bike
    hatch_daycare_egg(env.console, context);
    shiny_found = shiny_check_summary(env.console, context);
    if (shiny_found){
        return true;
    }

    // read stats
    AdvObservedPokemon observed_egg = read_summary(env.console, context, LANGUAGE);
    AdvRngFilters filters = observation_to_filters(observed_egg, EGG_STATS.base_stats, AdvRngMethod::Any);
    RNG_FILTERS.set(filters);

    // narrow down possible hits 
    auto elapsed_ms = std::chrono::duration_cast<Milliseconds>(elapsed);
    uint64_t advances_estimate = uint64_t(HELD_ADVANCES + 2 * elapsed_ms.count() / FRLG_FRAME_DURATION);
    env.log("Test pickup advances estimate: " + std::to_string(advances_estimate));    

    auto search_hits = get_egg_search_results(
        env.console, egg_searcher, filters,
        { current_seed }, { current_seed },
        HELD_ADVANCES, advances_radius, advances_estimate, HELD_CHECK_ADVANCES_RADIUS,
        PARENT_A, PARENT_B, COMPATIBILITY,
        EGG_STATS.gender_threshold, 0
    );

    std::vector<AdvRngState> held_hits;
    for (auto hit_pair : search_hits){
        held_hits.emplace_back(hit_pair.first);
    }
    std::sort(held_hits.begin(), held_hits.end());
    std::vector<AdvRngState>::iterator iter;
    iter = std::unique(held_hits.begin(), held_hits.begin() + held_hits.size());
    held_hits.resize(std::distance(held_hits.begin(), iter));
    HELD_CALIBRATION.set_hits(held_hits);

    bool finished = update_history(env.console, egg_uncertain_history, held_calibration_history, MAX_HISTORY_LENGTH, calibrations, held_hits, 1, 1);
    for (uint64_t i=0; i<candies_left; i++){
        if (finished){
            break;
        }

        bool failed = use_rare_candy(env.console, context, LANGUAGE, observed_egg, filters, EGG_STATS.base_stats, AdvRngMethod::Any, false, i == 0);
        if (failed){
            stats.errors++;
            send_program_recoverable_error_notification(
                env, NOTIFICATION_ERROR_RECOVERABLE,
                "Failed to use Rare Candy."
            ); 
        }
        
        RNG_FILTERS.set(filters);

        search_hits = get_egg_search_results(
            env.console, egg_searcher, filters,
            { current_seed }, { current_seed },
            HELD_ADVANCES, advances_radius, advances_estimate, HELD_CHECK_ADVANCES_RADIUS,
            PARENT_A, PARENT_B, COMPATIBILITY,
            EGG_STATS.gender_threshold, 0
        );
        held_hits.clear();
        for (auto hit_pair : search_hits){
            held_hits.emplace_back(hit_pair.first);
        }
        std::sort(held_hits.begin(), held_hits.end());
        iter = std::unique(held_hits.begin(), held_hits.begin() + held_hits.size());
        held_hits.resize(std::distance(held_hits.begin(), iter));
        HELD_CALIBRATION.set_hits(held_hits);   

        bool force_finish = (
                failed 
            ||  (i == (candies_left - 1))
            ||  all_indistinguishable(
                    search_hits, egg_searcher, 
                    EGG_STATS.gender_threshold,
                    PARENT_A, PARENT_B
                )
        );
        finished = update_history(
            env.console, egg_uncertain_history, 
            held_calibration_history, MAX_HISTORY_LENGTH, 
            calibrations, held_hits, 
            1, 1, force_finish
        );
    }

    env.log("RNG Search finished");
    if (search_hits.size() == 0){
        failed_searches++;
    }else{
        failed_searches = 0;
    }

    bool possibly_hit_held_frame = false;
    if (egg_uncertain_history.results.size()){
        for (AdvRngState rngstate : held_hits){
            if (
                rngstate.seed == TARGET_HELD_SEED
                && rngstate.advance == HELD_ADVANCES
            ){
                possibly_hit_held_frame = true;
                break;
            }
        }
    }

    if (held_calibration_history.results.empty()){
        return false;
    }

    bool definitely_hit_held_frame = false;
    if (egg_uncertain_history.results.size() == 0){
        for (size_t i=0; i<held_calibration_history.results.size(); i++){
            AdvRngState res = held_calibration_history.results[i];
            if (   res.seed == TARGET_HELD_SEED          
                && res.advance == HELD_ADVANCES
            ){
                definitely_hit_held_frame = true;
                // keep the calibrations that resulted in hitting the target held frame
                // and throw out the others
                calibrations = held_calibration_history.calibrations[i];
                HELD_CALIBRATION.set_calibrations(calibrations);
                held_calibration_history.results.clear();
                held_calibration_history.results.emplace_back(res);
                held_calibration_history.calibrations.clear();
                held_calibration_history.calibrations.emplace_back(calibrations);
                break;
            }
        }
        
    }
    

    if (definitely_hit_held_frame){
        env.log("Confirmed hit on the target held frame!");
    }else if(possibly_hit_held_frame){
        env.log("Possibly hit target held frame.");
    }else{
        env.log("Missed the target held frame.");
    }

    if (locked_in && !(definitely_hit_held_frame || possibly_hit_held_frame)){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "EggRng(): Missed held frame after saving. Restart the program after repeating the manual in-game setup.",
            env.console
        ); 
    }

    previously_hit_held_frame = definitely_hit_held_frame;

    return (locked_in && (definitely_hit_held_frame || possibly_hit_held_frame));
}


bool EggRng::pickup_frame_check(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context,
    EggRng_Descriptor::Stats& stats,
    RngUncertainHistory& egg_uncertain_history,
    RngCalibrationHistory& pickup_calibration_history,
    AdvRngEggSearcher& egg_searcher,
    RngCalibrations& calibrations,
    const uint64_t& candies_left,
    uint16_t& failed_searches,
    bool& shiny_found,
    const uint16_t& TARGET_HELD_SEED,
    const uint16_t& TARGET_PICKUP_SEED,
    const RngStats& EGG_STATS,
    AdvIVs& PARENT_A,
    AdvIVs& PARENT_B
){

    const uint16_t MAX_HISTORY_LENGTH = USE_TEACHY_TV ? 2 : 5;
    const uint64_t INITIAL_ADVANCES_RADIUS = USE_TEACHY_TV ? 4096 : 1024;

    uint64_t advances_radius = get_advances_radius(env.console, pickup_calibration_history, INITIAL_ADVANCES_RADIUS);

    pbf_mash_button(context, BUTTON_B, 5000ms); // return to overworld
    walk_from_pond_to_daycare_man(env.console, context);

    // egg already collected. Hatch...
    pbf_press_button(context, BUTTON_MINUS, 200ms, 800ms); // hop on bike
    hatch_daycare_egg(env.console, context);
    shiny_found = shiny_check_summary(env.console, context, 1);
    if (shiny_found){
        return true;
    }

    // ... read ...
    AdvObservedPokemon observed_egg = read_summary(env.console, context, LANGUAGE);
    AdvRngFilters filters = observation_to_filters(observed_egg, EGG_STATS.base_stats, AdvRngMethod::Any);
    RNG_FILTERS.set(filters);

    // ...and narrow down possible hits
    auto search_hits = get_egg_search_results(
        env.console, egg_searcher, filters,
        { TARGET_HELD_SEED }, { TARGET_PICKUP_SEED },
        HELD_ADVANCES, 0, PICKUP_ADVANCES, advances_radius,
        PARENT_A, PARENT_B, COMPATIBILITY,
        EGG_STATS.gender_threshold, 0
    );

    std::vector<AdvRngState> pickup_hits;
    for (auto hit_pair : search_hits){
        pickup_hits.emplace_back(hit_pair.second);
    }
    PICKUP_CALIBRATION.set_hits(pickup_hits);
    
    bool finished = update_history(env.console, egg_uncertain_history, pickup_calibration_history, MAX_HISTORY_LENGTH, calibrations, pickup_hits, 1);
    for (uint64_t i=0; i<candies_left; i++){
        if (finished){
            break;
        }

        bool failed = use_rare_candy(env.console, context, LANGUAGE, observed_egg, filters, EGG_STATS.base_stats, AdvRngMethod::Any, false, i == 0, 1);
        if (failed){
            stats.errors++;
            send_program_recoverable_error_notification(
                env, NOTIFICATION_ERROR_RECOVERABLE,
                "Failed to use Rare Candy."
            ); 
        }
        RNG_FILTERS.set(filters);

        search_hits = get_egg_search_results(
            env.console, egg_searcher, filters,
            { TARGET_HELD_SEED }, { TARGET_PICKUP_SEED },
            HELD_ADVANCES, 0, PICKUP_ADVANCES, advances_radius,
            PARENT_A, PARENT_B, COMPATIBILITY,
            EGG_STATS.gender_threshold, 0
        );
        pickup_hits.clear();
        for (auto hit_pair : search_hits){
            pickup_hits.emplace_back(hit_pair.second);
        }
        PICKUP_CALIBRATION.set_hits(pickup_hits);  

        bool force_finish = (
                failed 
            ||  (i == (candies_left - 1))
            ||  all_indistinguishable(
                    search_hits, egg_searcher, 
                    EGG_STATS.gender_threshold,
                    PARENT_A, PARENT_B
                )
        );
        finished = update_history(
            env.console, egg_uncertain_history, 
            pickup_calibration_history, MAX_HISTORY_LENGTH, 
            calibrations, pickup_hits, 
            1, 2, force_finish
        );
    }

    env.log("RNG Search finished");
    if (search_hits.size() == 0){
        failed_searches++;
    }else{
        failed_searches = 0;
    }

    if (pickup_calibration_history.results.empty()){
        return false;
    }

    auto newest_result = pickup_calibration_history.results.back();
    return (
        newest_result.seed == TARGET_PICKUP_SEED
        && newest_result.advance == PICKUP_ADVANCES
    );
    
}


void EggRng::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    /*
    * Settings: Text Speed fast
    */

    EggRng_Descriptor::Stats& stats = env.current_stats<EggRng_Descriptor::Stats>();

    home_black_border_check(env.console, context);

    // prepare database of base stats and gender thresholds
    RngStatsDatabase STATS_DATA("PokemonFRLG/BaseStats.json");

    RngStats EGG_STATS;
    try{
        EGG_STATS = STATS_DATA.get_throw(EGG_SPECIES.slug());        
    }catch (const InternalProgramError& err){
        env.log(err.message());
        env.log(EGG_SPECIES.slug());
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            err.message(),
            env.console
        ); 
    }

    // get the relevant encounter slots
    EncountersDatabase encounters_data(GAME_VERSION == GameVersion::firered ? "PokemonFRLG/EncounterSlotsFR.json" : "PokemonFRLG/EncounterSlotsLG.json");
    std::map<std::string, std::vector<AdvEncounterSlot>> location_map = encounters_data.get_throw("surfing");
    std::vector<AdvEncounterSlot> ENCOUNTER_SLOTS = location_map.find("four_island")->second;
    env.log("Pond Encounter slots");
    for (size_t i=0; i<ENCOUNTER_SLOTS.size(); i++){
        AdvEncounterSlot& slot = ENCOUNTER_SLOTS[i];
        env.log("   Slot " + std::to_string(i) + ": " + slot.species + " " + std::to_string(slot.minlevel) + "-" + std::to_string(slot.maxlevel));
    }    

    std::set<std::string> SPECIES_LIST;
    for (auto slot : ENCOUNTER_SLOTS){
        SPECIES_LIST.emplace(slot.species);
    }

    // held timings
    const uint16_t TARGET_HELD_SEED = parse_seed(env.console, HELD_SEED);
    const std::vector<uint16_t> HELD_SEED_VALUES = parse_seed_list(env.console, HELD_SEED_LIST);
    const int16_t HELD_SEED_POSITION = seed_position_in_list(TARGET_HELD_SEED, HELD_SEED_VALUES);
    if (HELD_SEED_POSITION == -1){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "EggRng(): Held Seed is missing from the list of nearby seeds.",
            env.console
        ); 
    }
    env.log("Target Held Seed Value: " + to_hex_string(TARGET_HELD_SEED));

    // pickup timings
    const uint16_t TARGET_PICKUP_SEED = parse_seed(env.console, PICKUP_SEED);
    const std::vector<uint16_t> PICKUP_SEED_VALUES = parse_seed_list(env.console, PICKUP_SEED_LIST);
    const int16_t PICKUP_SEED_POSITION = seed_position_in_list(TARGET_PICKUP_SEED, PICKUP_SEED_VALUES);
    if (PICKUP_SEED_POSITION == -1){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "EggRng(): Pickup Seed is missing from the list of nearby seeds.",
            env.console
        ); 
    }
    env.log("Target Pickup Seed Value: " + to_hex_string(TARGET_PICKUP_SEED));

    // searchers
    AdvRngWildSearcher wild_searcher(TARGET_HELD_SEED, HELD_ADVANCES, ENCOUNTER_SLOTS, AdvRngMethod::Any);
    AdvRngEggSearcher egg_searcher(TARGET_HELD_SEED, HELD_ADVANCES, TARGET_PICKUP_SEED, PICKUP_ADVANCES, AdvRngMethod::Any);

    ParentIVsRow& rowA = static_cast<ParentIVsRow&>(*PARENT_IVS.table()[0]);
    ParentIVsRow& rowB = static_cast<ParentIVsRow&>(*PARENT_IVS.table()[1]);
    AdvIVs PARENT_A = { rowA.hp, rowA.atk, rowA.def, rowA.spa, rowA.spd, rowA.spe };
    AdvIVs PARENT_B = { rowB.hp, rowB.atk, rowB.def, rowB.spa, rowB.spd, rowB.spe };

    AdvPokemonResult target_result = egg_searcher.generate_pokemon(PARENT_A, PARENT_B);
    RNG_TARGET.set_target(target_result, EGG_STATS.gender_threshold);
    env.log("Target PID: " + to_hex_string(target_result.pid));
    env.log("Target Nature: " + nature_to_string(target_result.nature));
    env.log("Target IVs (assuming Normal method):");
    env.log("HP: " + std::to_string(target_result.ivs.hp));
    env.log("Atk: " + std::to_string(target_result.ivs.attack));
    env.log("Def: " + std::to_string(target_result.ivs.defense));
    env.log("SpA: " + std::to_string(target_result.ivs.spatk));
    env.log("SpD: " + std::to_string(target_result.ivs.spdef));
    env.log("Spe: " + std::to_string(target_result.ivs.speed));

    Milliseconds launch_delay = INITIAL_LAUNCH_DELAY;

    RngUncertainHistory wild_uncertain_history;
    RngUncertainHistory egg_uncertain_history;
    RngCalibrationHistory wild_history; 
    RngCalibrationHistory held_calibration_history; 
    RngCalibrationHistory pickup_calibration_history; 

    RngCalibrations calibrations;
    if (STARTING_POINT == EggProgramState::pickup_calibration){
        calibrations = {
            PICKUP_CALIBRATION.seed_calibration / FRLG_FRAME_DURATION,
            PICKUP_CALIBRATION.csf_calibration,
            PICKUP_CALIBRATION.advances_calibration
        };
    }else{
        calibrations = {
            HELD_CALIBRATION.seed_calibration / FRLG_FRAME_DURATION,
            HELD_CALIBRATION.csf_calibration,
            HELD_CALIBRATION.advances_calibration
        };
    }

    env.log("Initial calibratons:");
    env.log("   Seed: " + std::to_string(calibrations.seed_offset) + " frames");
    env.log("   CSF: " + std::to_string(calibrations.csf_offset) + " frames");
    env.log("   In-Game: " + std::to_string(calibrations.ingame_offset) + " frames");

    bool shiny_found = false;

    bool previously_hit_held_frame = false;

    uint64_t balls_left = MAX_BALL_THROWS;
    uint64_t candies_left = MAX_RARE_CANDIES;
    uint16_t failed_searches = 0;
    uint16_t times_not_held = 0;

    EggProgramState program_state = STARTING_POINT;

    
    // main loop
    while (true){
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
        }

        if (pickup_calibration_history.results.size() > 0){
            env.log("Checking for nonshiny target hit...");
            if (have_hit_target(env, TARGET_HELD_SEED, HELD_ADVANCES, pickup_calibration_history.results.back())){
                env.log("Target Hit!");
                stats.nonshiny++;
                break;
            }
            env.log("Missed target.");
        }

        if (failed_searches >= 5){
            env.log("Failed to find any matches 5 times in a row");
            OperationFailedException::fire(
                ErrorReport::NO_ERROR_REPORT,
                "Failed to find any matches 5 times in a row. Check your seed and advances settings.",
                env.console
            ); 
            break;
        }

        if (stats.resets > MAX_RESETS){
            env.log("Max resets reached.");
            break;
        }

        bool hit_seed;
        bool held_finished;
        bool pickup_finished;

        WallClock timestamp; 

        uint16_t current_seed;

        uint64_t temp_balls_left;
        uint64_t temp_candies_left;

        switch (program_state){
        case EggProgramState::held_prep:
            prep_held_resets(env, context);
            egg_uncertain_history.results.clear();
            egg_uncertain_history.calibrations.clear();
            wild_uncertain_history.results.clear();
            wild_uncertain_history.calibrations.clear();
            program_state = EggProgramState::held_calibration;
            STARTING_POINT.set(EggProgramState::held_calibration);
            continue;

        case EggProgramState::held_calibration:
            current_seed = 0;
            temp_balls_left = balls_left;
            temp_candies_left = candies_left;
            hit_seed = reset_and_check_seed(
                env, context, stats, 
                wild_uncertain_history, egg_uncertain_history,
                wild_history, held_calibration_history,
                wild_searcher, calibrations,
                timestamp, current_seed,
                temp_balls_left, temp_candies_left, failed_searches, 
                times_not_held, shiny_found, previously_hit_held_frame,
                launch_delay,
                HELD_SEED_DELAY, HELD_SEED_VALUES, HELD_SEED_POSITION,
                SPECIES_LIST, STATS_DATA,
                false
            );

            if (!hit_seed) { continue; }

            held_finished = held_frame_check(
                env, context, stats,
                egg_uncertain_history, held_calibration_history,
                egg_searcher, calibrations, 
                timestamp, current_seed,
                candies_left, failed_searches, 
                shiny_found, previously_hit_held_frame,
                TARGET_HELD_SEED, 
                EGG_STATS, PARENT_A, PARENT_B
            );

            if (held_finished){
                env.log("Hit held frame after saving. Moving on to pickup frame calibration...");
                stats.balls += (balls_left - temp_balls_left);
                stats.candies += (candies_left - temp_candies_left);
                balls_left = temp_balls_left;
                candies_left = temp_candies_left;
                egg_uncertain_history.results.clear();
                egg_uncertain_history.calibrations.clear();
                wild_uncertain_history.results.clear();
                wild_uncertain_history.calibrations.clear();
                program_state = EggProgramState::pickup_calibration;
                STARTING_POINT.set(EggProgramState::pickup_calibration);
            }
            continue;

        case EggProgramState::pickup_calibration:
            current_seed = 0; // unused here
            hit_seed = reset_and_check_seed(
                env, context, stats, 
                wild_uncertain_history, egg_uncertain_history,
                wild_history, pickup_calibration_history,
                wild_searcher, calibrations, 
                timestamp, current_seed,
                balls_left, candies_left, failed_searches, 
                times_not_held, shiny_found, previously_hit_held_frame,
                launch_delay,
                PICKUP_SEED_DELAY, PICKUP_SEED_VALUES, PICKUP_SEED_POSITION,
                SPECIES_LIST, STATS_DATA,
                true
            );

            if (!hit_seed) { continue; }

            pickup_finished = pickup_frame_check(
                env, context, stats,
                egg_uncertain_history, pickup_calibration_history,
                egg_searcher, calibrations,
                candies_left, failed_searches, shiny_found,
                TARGET_HELD_SEED, TARGET_PICKUP_SEED,
                EGG_STATS, PARENT_A, PARENT_B
            );

            if (pickup_finished){
                env.log("Hit pickup frame!");
                program_state = EggProgramState::finished;
                STARTING_POINT.set(EggProgramState::held_prep);
            }
            continue;

        case EggProgramState::finished:
        default:
            if (GO_HOME_WHEN_DONE){
                pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
            }
            send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
        }
    }

    // just in case
    if (GO_HOME_WHEN_DONE){
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);

}



}
}
}
