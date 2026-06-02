/*  Starter RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_BattleDialogs.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_BattlePokemonDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_BattleLevelUpReader.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_RngNavigation.h"
#include "PokemonFRLG_HardReset.h"
#include "PokemonFRLG_StarterRng.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


StarterRng_Descriptor::StarterRng_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:StarterRng",
        Pokemon::STRING_POKEMON + " FRLG", "Starter RNG",
        "Programs/PokemonFRLG/StarterRng.html",
        "Automatically calibrate timings to hit a specific RNG target for FRLG starters.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct StarterRng_Descriptor::Stats : public StatsTracker{
    Stats()
        : resets(m_stats["Resets"])
        , shinies(m_stats["Shinies"])
        , nonshiny(m_stats["Non-Shiny Hits"])
        , wildshinies(m_stats["Wild Shinies"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Non-Shiny Hits", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Wild Shinies", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& nonshiny;
    std::atomic<uint64_t>& wildshinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> StarterRng_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

StarterRng::StarterRng()
    : m_calibration_displays(
        "<font size=4><b>Calibration Displays</b></font> — These will update automatically as the program runs"
    )
    , m_game_info(
        "<font size=4><b>Game Information</b></font>"
    )
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
    , m_target_settings(
        "<font size=4><b>Target Settings</b></font> — Get these from an RNG search tool"
    )
    , STARTER(
        "<b>Starter Species:</b>",
        {
            {Starter::bulbasaur, "bulbasaur", "Bulbasaur"},
            {Starter::squirtle, "squirtle", "Squirtle"},
            {Starter::charmander, "charmander", "Charmander"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Starter::bulbasaur
    )    
    , SEED(
        false,
        "<b>Target Seed:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "70FE", "70FE",
        true
    )
    , SEED_LIST(
        "<b>Nearby Seeds:</b><br>"
        "This box should contain a list of seeds (in order) around and including your target seed, with one seed on each line",
        LockMode::LOCK_WHILE_RUNNING,
        "D000\n199A\n77A1\nAABC\n280C\n70FE\nB573\n02F2\n8084\nA533\nED1E", 
        "D000\n199A\n77A1\nAABC\n280C\n70FE\nB573\n02F2\n8084\nA533\nED1E",
        true
    )
    , SEED_BUTTON(
        "<b>Seed Button:</b>",
        {
            {SeedButton::A, "A", "A"},
            {SeedButton::Start, "Start", "Start"},
            {SeedButton::L, "L", "L (L=A)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        SeedButton::A
    )
    , EXTRA_BUTTON(
        "<b>Extra Button:</b><br>"
        "Additional button presses that affect the seed.",
        {
            {BlackoutButton::None, "None", "None"},
            {BlackoutButton::L, "L", "Blackout L"},
            {BlackoutButton::R, "R", "Blackout R"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        BlackoutButton::None
    )
    , SEED_DELAY(
        "<b>Seed Delay Time (ms):</b><br>"
        "The delay between starting the game and advancing past the title screen. Set this to match your target seed.<br>"
        "<i>If using Ten Lines for seed info, select <b>Nintendo Switch 1</b> as your console even if using a Switch 2.</i><br>"
        "<b>Warning: values close to 30500ms can sometimes cause problems, and you may need to manually increase your initial seed calibration or pick a new target.</b>",
        LockMode::LOCK_WHILE_RUNNING,
        31338, 30400 // default, min
    )
    , ADVANCES(
        "<b>Advances:</b><br>"
        "The total number of RNG advances for your target.",
        LockMode::LOCK_WHILE_RUNNING,
        10000, 940, 1000000000 // default, min
    )
    , m_program_settings(
        "<font size=4><b>Program Settings</b></font>"
    )
    , MAX_RESETS(
        "<b>Max Resets:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        50, 0 // default, min
    )
    , IGNORE_WILD_SHINIES(
        "<b>Ignore wild shinies</b><br>"
        "Do not stop the program when a wild shiny is encountered.",
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
        "<b>Take Video:</b><br>"
        "Record a video when the shiny is found.", 
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
    PA_ADD_OPTION(RNG_CALIBRATION);
    PA_ADD_OPTION(m_game_info);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(m_target_settings);
    PA_ADD_OPTION(STARTER);
    PA_ADD_OPTION(SEED);
    PA_ADD_OPTION(SEED_LIST);
    PA_ADD_OPTION(SEED_BUTTON);
    PA_ADD_OPTION(EXTRA_BUTTON);
    PA_ADD_OPTION(SEED_DELAY);
    PA_ADD_OPTION(ADVANCES);
    PA_ADD_OPTION(m_program_settings);
    PA_ADD_OPTION(MAX_RESETS);
    PA_ADD_OPTION(PROFILE);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}



bool StarterRng::have_hit_target(SingleSwitchProgramEnvironment& env, const uint32_t& TARGET_SEED, const AdvRngState& hit){
    return (hit.seed == TARGET_SEED) && (hit.advance == ADVANCES);
}



bool StarterRng::walk_to_rival_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // return to the overworld
    pbf_mash_button(context, BUTTON_B, 5000ms);
    int num_steps_to_the_left;
    switch(STARTER){
    case Starter::bulbasaur:
        num_steps_to_the_left = 2;
        break;
    case Starter::squirtle:
        num_steps_to_the_left = 3;
        break;
    case Starter::charmander:
        num_steps_to_the_left = 4;
        break;
    default:
        num_steps_to_the_left = 2;
    }

    // dodge rival
    pbf_move_left_joystick(context, {0, -1}, 40ms, 460ms);
    pbf_move_left_joystick(context, {0, -1}, 100ms, 400ms);

    // line up with the doorway
    pbf_move_left_joystick(context, {-1, 0}, 40ms, 460ms); // pivot left
    for (int i=0; i<num_steps_to_the_left; i++){
        pbf_move_left_joystick(context, {-1, 0}, 100ms, 400ms);
    }
    
    // walk down and trigger battle
    BlackScreenWatcher black_screen(COLOR_RED);
    context.wait_for_all_requests();
    int ret =  run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context) {
            for (int i=0; i<5; i++){
                ssf_press_left_joystick(context, {0, -1}, 0ms, 20000ms, 0ms);
                ssf_mash1_button(context, BUTTON_B, 20000ms); 
            }
        },
        { black_screen }
    );

    bool failed = ret < 0;
    if (failed){
        send_program_recoverable_error_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            "walk_to_rival_battle(): failed to initiate battle."
        ); 
    }
    return failed;
}

bool StarterRng::auto_battle_rival(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    AdvObservedPokemon& pokemon,
    AdvRngFilters& filters,
    const BaseStats& BASE_STATS
){
    Pokemon::EVs evyield = {0, 0, 0, 0, 0, 0};
    switch(STARTER){
        case Starter::bulbasaur:
            evyield.speed = 1;   // from charmander
            break;
        case Starter::squirtle:
            evyield.spatk = 1;   // from bulbasaur
            break;
        case Starter::charmander:
            evyield.defense = 1; // from squirtle
    }

    // detect the battle menu
    BattleMenuWatcher battle_ready(COLOR_RED);
    context.wait_for_all_requests();
    int ret1 = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context) {
            pbf_mash_button(context, BUTTON_B, 30s);
        },
        { battle_ready }
    );
    if (ret1 < 0){
        send_program_recoverable_error_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            "auto_battle_rival(): failed to detect the battle menu."
        );
        return true;
    }
    env.log("Battle started. Using first move...");

    // perform the first move and get through Oak's dialogue,
    // which messes up most detectors when it dims the screen
    pbf_mash_button(context, BUTTON_A, 1000ms); // execute first move
    context.wait_for_all_requests();
    int ret2 = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context) {
            pbf_mash_button(context, BUTTON_B, 30s);
        },
        { battle_ready }
    );
    if (ret2 < 0){
        send_program_recoverable_error_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            "auto_battle_rival(): failed to detect the battle menu."
        );
        return true;
    }
    env.log("Oak tutorial dialogue finished. Mashing A...");

    // mash A until somebody faints
    BattleOpponentFaintWatcher player_won(COLOR_RED);
    BattleFaintWatcher player_lost(COLOR_RED);
    context.wait_for_all_requests();
    int ret3 = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context) {
            pbf_mash_button(context, BUTTON_A, 300s);
        },
        { player_won, player_lost }
    );

    switch(ret3){
    case 0:
        env.log("Won battle against rival. Watching for level-up stats...");
        break;
    case 1:
        env.log("Lost battle against rival.");
        pbf_mash_button(context, BUTTON_B, 20s); // exit battle and dialogue
        return false;
    default:
        send_program_recoverable_error_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            "auto_battle_rival(): no fainting detected with 5 minutes."
        );
        return true;
    }

    // slowly advance dialog until level-up stats are visible
    BattleLevelUpWatcher level_up_stats(COLOR_RED, BattleLevelUpDialog::stats, LANGUAGE);
    BlackScreenWatcher black_screen(COLOR_RED);
    context.wait_for_all_requests();
    int ret4 = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context) {
            for(int i=0; i<60; i++){
                pbf_press_button(context, BUTTON_A, 200ms, 1800ms);
            }
        },
        { level_up_stats, black_screen }
    );

    switch(ret4){
    case 0:
        env.log("Level-up stats detected.");
        break;
    case 1:
        env.log("Battle exited without detecting level-up stats");
        return true; // will cause issues with keeping track of level and EVs
    default:
        env.log("auto_battle_rival(): no recognized state within 2 minutes of winning battle.");
        return true;
    }

    // read stats
    BattleLevelUpReader reader(COLOR_RED);
    VideoOverlaySet overlays(env.console.overlay());
    reader.make_overlays(overlays);

    env.log("Reading stats...");
    VideoSnapshot screen = env.console.video().snapshot();
    StatReads stats = reader.read_stats(env.logger(), screen);    

    update_filters(filters, pokemon, stats, evyield, BASE_STATS);
    RNG_FILTERS.set(filters);   

    // exit battle
    pbf_mash_button(context, BUTTON_B, 20s);
    context.wait_for_all_requests();

    return false;
}



bool StarterRng::walk_to_route1_from_lab(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // exit the lab
    env.log("Exiting the lab...");
    BlackScreenWatcher black_screen(COLOR_RED);
    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context) {
            pbf_move_left_joystick(context, {0, -1}, 10s, 0ms);
        },
        { black_screen }
    );

    if (ret < 0){
        send_program_recoverable_error_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            "walk_to_route1_from_lab(): failed to exit lab."
        );
        return true;
    }

    env.log("Lab exited. Walking to Route 1...");
    pbf_wait(context, 5000ms);
    pbf_move_left_joystick(context, {-1, 0}, 1280ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 3150ms, 300ms);
    pbf_move_left_joystick(context, {+1, 0}, 330ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 720ms, 300ms);
    context.wait_for_all_requests();
    return false;
}

bool StarterRng::walk_to_route1_from_home(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // exit the house
    env.log("Exiting the house...");
    BlackScreenWatcher black_screen(COLOR_RED);
    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context) {
            pbf_move_left_joystick(context, {0, -1}, 1000ms, 300ms);
            pbf_move_left_joystick(context, {-1, 0}, 900ms, 300ms);
            pbf_move_left_joystick(context, {0, -1}, 1000ms, 300ms);
        },
        { black_screen }
    );

    if (ret < 0){
        send_program_recoverable_error_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            "walk_to_route1_from_home(): failed to exit the house."
        );
        return true;
    }

    env.log("House exited. Walking to Route 1...");
    pbf_wait(context, 5s);
    pbf_move_left_joystick(context, {+1, 0}, 1370ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 1450ms, 300ms);
    pbf_move_left_joystick(context, {+1, 0}, 300ms,  300ms);
    pbf_move_left_joystick(context, {0, +1}, 1250ms, 300ms);

    return false;
}

int StarterRng::autolevel_on_route1(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    AdvObservedPokemon& pokemon,
    AdvRngFilters& filters,
    const BaseStats& BASE_STATS
){
    Pokemon::EVs evyield = {0, 0, 0, 0, 0, 0};
    Pokemon::StatReads stats;

    env.log("Arrived at Route 1.");
    bool leftright = true;
    context.wait_for_all_requests();

    while (true){
        // trigger encounter
        env.log("Triggering wild encounters...");
        int ret = grass_spin(env.console, context, leftright);
        if (ret < 0){
            send_program_recoverable_error_notification(
                env, NOTIFICATION_ERROR_RECOVERABLE,
                "autolevel_on_route1(): failed to trigger encounter."
            );
            return -1;
        }

        if (ret == 1){ // shiny
            return 1;
        }

        // auto battle
        BattleResult ret2 = spam_first_move(env.console, context);

        BattleLevelUpWatcher level_up(COLOR_RED, BattleLevelUpDialog::stats, LANGUAGE);
        BlackScreenWatcher black_screen(COLOR_RED);
        VideoSnapshot screen;
        int ret3;
        bool failed;
        BattleLevelUpReader reader;          

        switch (ret2){
        case BattleResult::opponentfainted:
            env.log("Opponent fainted.");
            evyield.speed++; // always rattata or pidgey
            leftright = !leftright;

            context.wait_for_all_requests();
            ret3 = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context) {
                    for (int i=0; i<5; i++){
                        pbf_press_button(context, BUTTON_B, 200ms, 1800ms);
                    }
                },
                { level_up, black_screen }
            );

            switch (ret3){
            case 0:
                env.log("Level-up stats detected. Reading stats...");
                screen = env.console.video().snapshot();      
                stats = reader.read_stats(env.logger(), screen);
                update_filters(filters, pokemon, stats, evyield, BASE_STATS);
                RNG_FILTERS.set(filters);
                exit_wild_battle(env.console, context, false, true);
                return 0;
            case -1:
                exit_wild_battle(env.console, context, false, true);
            default:
                pbf_wait(context, 1000ms);
                context.wait_for_all_requests();
                continue;
            }
        case BattleResult::playerfainted:
            env.log("Pokemon fainted. Mashing B through dialogues...");
            pbf_mash_button(context, BUTTON_B, 30s); // skip through a few transitions and lots of dialogue
            failed = walk_to_route1_from_home(env, context);
            if (failed){
                return -1;
            }
            context.wait_for_all_requests();
            leftright = true;
            continue;
        case BattleResult::outofpp:
        case BattleResult::unknown:
        default:
            return -1;
        }
    }
}

void StarterRng::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    /*
    * Settings: Text Speed fast
    */

    StarterRng_Descriptor::Stats& stats = env.current_stats<StarterRng_Descriptor::Stats>();

    home_black_border_check(env.console, context);

    RNG_FILTERS.reset();
    RNG_CALIBRATION.reset_hits();

    const uint16_t TARGET_SEED = parse_seed(env.console, SEED);
    const std::vector<uint16_t> SEED_VALUES = parse_seed_list(env.console, SEED_LIST);
    const int16_t SEED_POSITION = seed_position_in_list(TARGET_SEED, SEED_VALUES);

    if (SEED_POSITION == -1){
        throw UserSetupError(env.console, "The target Seed is missing from the list of nearby seeds.");
    }

    BaseStats BASE_STATS;
    switch (STARTER){
    case Starter::bulbasaur:
        BASE_STATS = { 45, 49, 49, 65, 65, 45 };
        break;
    case Starter::squirtle:
        BASE_STATS = { 44, 48, 65, 50, 64, 43 };
        break;
    case Starter::charmander:
        BASE_STATS = { 39, 52, 43, 60, 50, 65 };
        break;
    default:
        break;
    }

    static const int64_t FIXED_SEED_OFFSET = -845; // milliseconds, approximate
    static const int64_t FIXED_ADVANCES_OFFSET = 160; // frames, approximate

    static const uint64_t CONTINUE_SCREEN_FRAMES = 200;

    static const double SEED_BUMPS[] = {0, 1, -1, 2, -2};

    static const uint64_t INITIAL_ADVANCES_RADIUS = 1024;

    static const uint8_t MAX_HISTORY_LENGTH = 10;

    static const int16_t GENDER_THRESHOLD = 30;

    static const std::set<std::string> SPECIES_LIST = { "bulbasaur", "squirtle", "charmander" };

    env.log("RNG Target: " + std::to_string(STARTER.current_value()));
    env.log("Target Seed: " + to_hex_string(TARGET_SEED));
    env.log("Target Advances: " + std::to_string(ADVANCES));

    AdvRngSearcher searcher(TARGET_SEED, ADVANCES, AdvRngMethod::Method1);
    AdvPokemonResult target_result = searcher.generate_pokemon();
    RNG_TARGET.set_target(target_result, GENDER_THRESHOLD);
    env.log("Target PID: " + to_hex_string(target_result.pid));
    env.log("Target Nature: " + nature_to_string(target_result.nature));
    env.log("Target IVs:");
    env.log("   HP: " + std::to_string(target_result.ivs.hp));
    env.log("   Atk: " + std::to_string(target_result.ivs.attack));
    env.log("   Def: " + std::to_string(target_result.ivs.defense));
    env.log("   SpA: " + std::to_string(target_result.ivs.spatk));
    env.log("   SpD: " + std::to_string(target_result.ivs.spdef));
    env.log("   Spe: " + std::to_string(target_result.ivs.speed));
    
    RngCalibrations calibrations = {
        RNG_CALIBRATION.seed_calibration / FRLG_FRAME_DURATION,
        RNG_CALIBRATION.csf_calibration,
        RNG_CALIBRATION.advances_calibration
    };
    env.log("Initial Seed calibration (frames): " + std::to_string(calibrations.seed_offset));
    env.log("Initial CSF calibration (frames): " + std::to_string(calibrations.csf_offset));
    env.log("Initial In-game calibration (frames x2): " + std::to_string(calibrations.ingame_offset));

    Milliseconds launch_delay = INITIAL_LAUNCH_DELAY;

    RngAdvanceHistory advance_history;
    RngCalibrationHistory calibration_history; 

    bool wildshiny_found = false;

    uint16_t failed_searches = 0;

    while (true){
        if (calibration_history.results.size() > 0){
            env.log("Checking for nonshiny target hit...");
            if (have_hit_target(env, TARGET_SEED, calibration_history.results.back())){
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

        if (wildshiny_found){
            break;
        }

        send_program_status_notification(
            env, NOTIFICATION_STATUS_UPDATE,
            "Calibrating."
        );
        env.update_stats();


        uint64_t advances_radius = get_advances_radius(env.console, calibration_history, INITIAL_ADVANCES_RADIUS);

        if (calibration_history.results.size() > 0){
            calibrations = get_calibrations(env.console, calibration_history, SEED_VALUES, SEED_POSITION, ADVANCES);
        }

        // if previous resets had uncertain advances, slightly modify the seed delay to try to hit a different target
        double seed_bump = SEED_BUMPS[advance_history.results.size() % 5];
        calibrations.seed_offset += seed_bump;

        uint64_t ingame_advances = ADVANCES - CONTINUE_SCREEN_FRAMES;

        RngTimings timings = prepare_timings(
            env.console, PokemonFRLG_RngTarget::starters,
            SEED_DELAY, CONTINUE_SCREEN_FRAMES, ingame_advances,
            false, calibrations,
            FIXED_SEED_OFFSET, FIXED_ADVANCES_OFFSET
        );

        env.log("Resetting Game...");
        reset_and_perform_blind_sequence(
            env.console, context, PokemonFRLG_RngTarget::starters, 
            SEED_BUTTON, EXTRA_BUTTON, timings,
            launch_delay, false, PROFILE
        );
        stats.resets++; 

        RNG_FILTERS.reset();
        RNG_CALIBRATION.set_calibrations(calibrations);
        RNG_CALIBRATION.reset_hits();

        bool shiny_found = check_for_shiny(env.console, context, PokemonFRLG_RngTarget::starters);

        if (shiny_found){
            env.log("Shiny found!");
            stats.shinies++;
            RNG_CALIBRATION.hits.set("Shiny!");
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

        // Stage 1: initial search -- starter received
        AdvObservedPokemon pokemon = read_summary(env.console, context, LANGUAGE, SPECIES_LIST);
        AdvRngFilters filters = observation_to_filters(pokemon, BASE_STATS);
        RNG_FILTERS.set(filters);

        std::vector<AdvRngState> search_hits = get_search_results(env.console, searcher, filters, SEED_VALUES, ADVANCES, advances_radius, GENDER_THRESHOLD);
        RNG_CALIBRATION.set_hits(search_hits);        
        bool finished = update_history(
            env.console, advance_history, calibration_history, 
            MAX_HISTORY_LENGTH, calibrations, search_hits, 1
        );
        if (finished){
            env.log("RNG search finished.");
            if (search_hits.size() == 0){
                failed_searches++;
            }else{
                failed_searches = 0;
            }
            continue;
        }

        // Stage 2: first search update -- post-rival-battle
        bool failed = walk_to_rival_battle(env, context);
        if (failed){
            stats.errors++;
            continue; // reset game
        }

        failed = auto_battle_rival(env, context, pokemon, filters, BASE_STATS);
        if (failed){
            stats.errors++;
            continue; // reset game
        }
        if (pokemon.level.size() > 1){
            search_hits = get_search_results(env.console, searcher, filters, SEED_VALUES, ADVANCES, advances_radius, GENDER_THRESHOLD);
            RNG_CALIBRATION.set_hits(search_hits);        
            env.log("Number of search hits: " + std::to_string(search_hits.size()));
            finished = update_history(
                env.console, advance_history, calibration_history, 
                MAX_HISTORY_LENGTH, calibrations, search_hits, 5
            );            
            if (finished){
                env.log("RNG search finished.");
                if (search_hits.size() == 0){
                    failed_searches++;
                }else{
                    failed_searches = 0;
                }
                continue;
            }
        }

        // Stage 3: subsequent search updates -- leveling up from wild encounters
        failed = walk_to_route1_from_lab(env, context);
        if (failed){
            stats.errors++;
            continue; // reset game
        }

        auto num_levels = pokemon.level.size();
        uint16_t MAX_LEVELS = 3;
        while(true){
            if (num_levels > MAX_LEVELS){
                env.log("RNG search not complete after 3 level-ups.");
                finished = update_history(
                    env.console, advance_history, calibration_history, 
                    MAX_HISTORY_LENGTH, calibrations, search_hits, 5, 2, true
                );         
                break;
            }

            env.log("Level: " + std::to_string(4 + pokemon.level.size()));
            env.log("Speed EVs: " + std::to_string(pokemon.evs.back().speed));

            int ret2 = autolevel_on_route1(env, context, pokemon, filters, BASE_STATS);
            if (ret2 < 0){
                stats.errors++;
                break;
            }else if(ret2 == 1){
                env.log("Wild shiny found!");
                stats.wildshinies++;
                send_program_notification(
                    env,
                    NOTIFICATION_SHINY,
                    COLOR_YELLOW,
                    "Wild Shiny found!",
                    {}, "",
                    env.console.video().snapshot(),
                    true
                );
                if (TAKE_VIDEO){
                    pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 0ms);
                }
                if (!IGNORE_WILD_SHINIES){
                    wildshiny_found = true;
                    break;
                }
            }
            
            // force the search to finish after enough level-ups 
            if (pokemon.level.size() > num_levels){
                num_levels = pokemon.level.size();
                search_hits = get_search_results(env.console, searcher, filters, SEED_VALUES, ADVANCES, advances_radius, GENDER_THRESHOLD);
                RNG_CALIBRATION.set_hits(search_hits);
                env.log("Number of search hits: " + std::to_string(search_hits.size()));
                update_history(
                    env.console, advance_history, calibration_history, 
                    MAX_HISTORY_LENGTH, calibrations, search_hits, 5, 2, true
                );
                env.log("RNG search finished.");
                if (search_hits.size() == 0){
                    failed_searches++;
                }else{
                    failed_searches = 0;
                }
                break;
            }
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
