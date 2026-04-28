/*  Gift RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <algorithm>
#include <sstream>
#include "CommonTools/Random.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_PartyDialogs.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_SummaryDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_PartyMenuDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_BagDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_PartyLevelUpReader.h"
#include "PokemonFRLG/Inference/PokemonFRLG_StatsReader.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_BlindNavigation.h"
#include "PokemonFRLG_RngNavigation.h"
#include "PokemonFRLG_HardReset.h"
#include "PokemonFRLG_GiftRng.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


GiftRng_Descriptor::GiftRng_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:GiftRng",
        Pokemon::STRING_POKEMON + " FRLG", "Gift RNG",
        "Programs/PokemonFRLG/GiftRng.html",
        "Automatically calibrate timings to hit a specific RNG target for FRLG gift " + STRING_POKEMON,
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct GiftRng_Descriptor::Stats : public StatsTracker{
    Stats()
        : resets(m_stats["Resets"])
        , shinies(m_stats["Shinies"])
        , nonshiny(m_stats["Non-Shiny Hits"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Non-Shiny Hits", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& nonshiny;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> GiftRng_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

GiftRng::GiftRng()
    : LANGUAGE(
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
    , TARGET(
        "<b>Target:</b><br>",
        {
            {PokemonFRLG_RngTarget::magikarp, "magikarp", "Magikarp"},
            {PokemonFRLG_RngTarget::hitmonchan, "hitmonchan", "Hitmonchan"},
            {PokemonFRLG_RngTarget::hitmonlee, "hitmonlee", "Hitmonlee"},
            {PokemonFRLG_RngTarget::eevee, "eevee", "Eevee"},
            {PokemonFRLG_RngTarget::lapras, "lapras", "Lapras"},
            {PokemonFRLG_RngTarget::omanyte, "omanyte", "Omanyte"},
            {PokemonFRLG_RngTarget::kabuto, "kabuto", "Kabuto"},
            {PokemonFRLG_RngTarget::aerodactyl, "aerodactyl", "Aerodactyl"},
            {PokemonFRLG_RngTarget::gamecornerabra, "gamecornerabra", "Game Corner Abra"},
            {PokemonFRLG_RngTarget::gamecornerclefairy, "gamecornerclefairy", "Game Corner Clefairy"},
            {PokemonFRLG_RngTarget::gamecornerdratini, "gamecornerdratini", "Game Corner Dratini"},
            {PokemonFRLG_RngTarget::gamecornerscyther, "gamecornerscyther", "Game Corner Scyther"},
            {PokemonFRLG_RngTarget::gamecornerpinsir, "gamecornerpinsir", "Game Corner Pinsir"},
            {PokemonFRLG_RngTarget::gamecornerporygon, "gamecornerporygon", "Game Corner Porygon"},
            {PokemonFRLG_RngTarget::togepi, "togepi", "Togepi"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        PokemonFRLG_RngTarget::magikarp
    )    
    , MAX_RESETS(
        "<b>Max Resets:</b><br>",
        LockMode::UNLOCK_WHILE_RUNNING,
        50, 0 // default, min
    )
    , MAX_RARE_CANDIES(
        "<b>Max Rare Candies:</b><br>"
        "The number of rare candies in your bag. Make sure these are at the top position of the bag.<br>"
        "Rare candies used during calibration will be restored after resetting.",
        LockMode::UNLOCK_WHILE_RUNNING,
        0, 0, 999 // default, min, max
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
        "<b>Seed Button:</b><br>",
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
        "<b>Seed Delay Time (ms):</b><br>The delay between starting the game and advancing past the title screen. Set this to match your target seed.",
        LockMode::LOCK_WHILE_RUNNING,
        31338, 28000 // default, min
    )
    , ADVANCES(
        "<b>Advances:</b><br>The total number of RNG advances for your target.<br>This should be the combined amount of continue screen and in-game advances.",
        LockMode::LOCK_WHILE_RUNNING,
        10000, 600, 1000000000 // default, min
    )
    // , CONTINUE_SCREEN_FRAMES(
    //     "<b>Continue Screen Frames:</b><br>The number of RNG advances to pass on the continue screen.<br>This should be less than the total number of advances above.",
    //     LockMode::LOCK_WHILE_RUNNING,
    //     1000, 192 // default, min
    // )
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
    PA_ADD_OPTION(RNG_FILTERS);
    PA_ADD_OPTION(RNG_CALIBRATION);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(TARGET);
    PA_ADD_OPTION(MAX_RESETS);
    PA_ADD_OPTION(MAX_RARE_CANDIES);
    PA_ADD_OPTION(SEED);
    PA_ADD_OPTION(SEED_LIST);
    PA_ADD_OPTION(SEED_BUTTON);
    PA_ADD_OPTION(EXTRA_BUTTON);
    PA_ADD_OPTION(SEED_DELAY);
    PA_ADD_OPTION(ADVANCES);
    // PA_ADD_OPTION(CONTINUE_SCREEN_FRAMES);
    PA_ADD_OPTION(USE_TEACHY_TV);
    PA_ADD_OPTION(PROFILE);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}



bool GiftRng::have_hit_target(SingleSwitchProgramEnvironment& env, const uint32_t& TARGET_SEED, const AdvRngState& hit){
    return (hit.seed == TARGET_SEED) && (hit.advance == ADVANCES);
}

AdvObservedPokemon GiftRng::read_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // assumes we're already on the first summary page
    PokemonFRLG_Stats stats;
    StatsReader reader(COLOR_RED);

    env.log("Reading Page 1 (Name, Level, Nature, Gender)...");
    VideoSnapshot screen1 = env.console.video().snapshot();
    reader.read_page1(env.logger(), LANGUAGE, screen1, stats);

    SummaryPage2Watcher page_two(COLOR_RED);
    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context) {
            for (int i=0; i<5; i++){
                pbf_press_dpad(context, DPAD_RIGHT, 200ms, 1800ms);
            }
        },
        { page_two }
    );

    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "read_summary(): Failed to detect second summary screen.",
            env.console
        ); 
    }

    env.log("Reading Page 2 (Stats)...");
    VideoSnapshot screen2 = env.console.video().snapshot();
    reader.read_page2(env.logger(), screen2, stats);

    StatReads statreads = {
        static_cast<int16_t>(stats.hp.value_or(0)),
        static_cast<int16_t>(stats.attack.value_or(0)),
        static_cast<int16_t>(stats.defense.value_or(0)),
        static_cast<int16_t>(stats.sp_attack.value_or(0)),
        static_cast<int16_t>(stats.sp_defense.value_or(0)),
        static_cast<int16_t>(stats.speed.value_or(0))
    };

    AdvGender gender;
    switch(stats.gender.value_or(SummaryGender::Genderless)){
    case SummaryGender::Male:
        gender = AdvGender::Male;
        break;
    case SummaryGender::Female:
        gender = AdvGender::Female;
        break;
    default:
        gender = AdvGender::Any;
        break;
    }

    AdvObservedPokemon pokemon = {
        gender,
        string_to_nature(stats.nature),
        AdvAbility::Any,
        { uint8_t(stats.level.value_or(5)) },
        { statreads },
        { {0,0,0,0,0,0} },
        AdvShinyType::Any
    };

    return pokemon;
}

bool GiftRng::use_rare_candy(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context,
    AdvObservedPokemon& pokemon,
    AdvRngFilters& filters,
    const BaseStats& BASE_STATS,
    bool first
){
    // navigate to the bag (only needed for the first use)
    if (first){
        open_bag_from_overworld(env.console, context);
        // move left to the correct pocket (in case Teachy TV was used)
        pbf_move_left_joystick(context, {-1, 0}, 200ms, 800ms);
        pbf_move_left_joystick(context, {-1, 0}, 200ms, 800ms);
    }

    // use rare candy and watch for the party screen
    PartyMenuWatcher party_menu(COLOR_RED);
    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context) {
            for (int i=0; i<5; i++){
                pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
            }
        },
        { party_menu }
    );
    if (ret < 0){
        send_program_recoverable_error_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            "use_rare_candy(): failed to detect party menu."
        ); 
        return true;
    }

    // select the last party slot (unknown how full the party is, so we can't detect a particular slot)
    // only needed on the first use
    if (first){
        context.wait_for_all_requests();
        pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
        pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    }

    // watch for level up stats
    PartyLevelUpWatcher level_up(COLOR_RED, PartyLevelUpDialog::stats);
    context.wait_for_all_requests();
    int ret2 = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context) {
            for (int i=0; i<30; i++){
                pbf_press_button(context, BUTTON_A, 200ms, 800ms);
            }
        },
        { level_up }
    );
    if (ret2 < 0){
        send_program_recoverable_error_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            "use_rare_candy(): failed to detect level-up stats."
        ); 
        return true;
    }

    PartyLevelUpReader reader(COLOR_RED);
    VideoOverlaySet overlays(env.console.overlay());
    reader.make_overlays(overlays);

    env.log("Reading stats...");
    VideoSnapshot screen = env.console.video().snapshot();
    StatReads stats = reader.read_stats(env.logger(), screen);    

    update_filters(filters, pokemon, stats, {}, BASE_STATS);
    RNG_FILTERS.set(filters);   

    // return to the bag (possibly learning a move, but trying to preven evolution)
    int attempts = 0;
    while (true){
        if (attempts > 5){
            send_program_recoverable_error_notification(
                env, NOTIFICATION_ERROR_RECOVERABLE,
                "use_rare_candy(): failed to return to bag menu in 5 attempts."
            );
            return true;
        }
        BagWatcher bag_menu(COLOR_RED);
        PartyMoveLearnWatcher move_learn(COLOR_RED);
        context.wait_for_all_requests();
        int ret3 = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context) {
                for (int i=0; i<15; i++){
                    pbf_press_button(context, BUTTON_B, 200ms, 1800ms);
                }
            },
            { bag_menu, move_learn }
        );
        attempts++;
        switch (ret3){
        case 0:
            env.log("Returned to bag.");
            return false;
        case 1:
            env.log("Move learn opportunity detected.");
            // don't learn move
            pbf_press_button(context, BUTTON_B, 200ms, 1800ms);
            pbf_press_button(context, BUTTON_A, 200ms, 1800ms);
            continue;
        default:
            send_program_recoverable_error_notification(
                env, NOTIFICATION_ERROR_RECOVERABLE,
                "use_rare_candy(): failed to return to bag menu."
            ); 
            return true;
        }
    }
}


void GiftRng::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    /*
    * Settings: Text Speed fast
    */

    GiftRng_Descriptor::Stats& stats = env.current_stats<GiftRng_Descriptor::Stats>();

    home_black_border_check(env.console, context);

    RNG_FILTERS.reset();
    RNG_CALIBRATION.reset();

    const uint16_t TARGET_SEED = parse_seed(env.console, SEED);
    const std::vector<uint16_t> SEED_VALUES = parse_seed_list(env.console, SEED_LIST);
    const int16_t SEED_POSITION = seed_position_in_list(TARGET_SEED, SEED_VALUES);

    if (SEED_POSITION == -1){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "GiftRng(): Target Seed is missing from the list of nearby seeds.",
            env.console
        ); 
    }

    env.log("Target Seed Value: " + std::to_string(TARGET_SEED));

    BaseStats BASE_STATS;
    int16_t GENDER_THRESHOLD = -1;
    switch (TARGET){
    case PokemonFRLG_RngTarget::magikarp:
        BASE_STATS = { 20, 10, 55, 15, 20, 80 };
        GENDER_THRESHOLD = 126;
        break;
    case PokemonFRLG_RngTarget::hitmonchan:
        BASE_STATS = { 50, 105, 79, 35, 110, 76 };
        GENDER_THRESHOLD = -1;
        break;
    case PokemonFRLG_RngTarget::hitmonlee:
        BASE_STATS = { 50, 120, 53, 35, 110, 87 };
        GENDER_THRESHOLD = -1;
        break;
    case PokemonFRLG_RngTarget::eevee:
        BASE_STATS = { 55, 55, 50, 45, 65, 55 };
        GENDER_THRESHOLD = 30;
        break;
    case PokemonFRLG_RngTarget::lapras:
        BASE_STATS = { 130, 85, 80, 85, 95, 60 };
        GENDER_THRESHOLD = 126;
        break;
    case PokemonFRLG_RngTarget::omanyte:
        BASE_STATS = { 35, 40, 100, 90, 55, 35 };
        GENDER_THRESHOLD = 30;
        break;
    case PokemonFRLG_RngTarget::kabuto:
        BASE_STATS = { 30, 80, 90, 55, 45, 55 };
        GENDER_THRESHOLD = 30;
        break;
    case PokemonFRLG_RngTarget::aerodactyl:
        BASE_STATS = { 80, 105, 65, 60, 75, 130 };
        GENDER_THRESHOLD = 30;
        break;
    case PokemonFRLG_RngTarget::gamecornerabra:
        BASE_STATS = { 25, 20, 15, 105, 55, 90 };
        GENDER_THRESHOLD = 63;
        break;
    case PokemonFRLG_RngTarget::gamecornerclefairy:
        BASE_STATS = { 70, 45, 48, 60, 65, 35 };
        GENDER_THRESHOLD = 190;
        break;
    case PokemonFRLG_RngTarget::gamecornerdratini:
        BASE_STATS = { 41, 64, 45, 50, 50, 50 };
        GENDER_THRESHOLD = 126;
        break;
    case PokemonFRLG_RngTarget::gamecornerscyther:
        BASE_STATS = { 70, 110, 80, 55, 80, 105 };
        GENDER_THRESHOLD = 126;
        break;
    case PokemonFRLG_RngTarget::gamecornerpinsir:
        BASE_STATS = { 65, 125, 100, 55, 70, 85 };
        GENDER_THRESHOLD = 126;
        break;
    case PokemonFRLG_RngTarget::gamecornerporygon:
        BASE_STATS = { 65, 60, 70, 85, 75, 40 };
        GENDER_THRESHOLD = -1;
        break;
    case PokemonFRLG_RngTarget::togepi:
        BASE_STATS = { 35, 20, 65, 40, 65, 20 };
        GENDER_THRESHOLD = 30;
        break; 
    default:
        break;
    }

    const double FRAMERATE = 59.999977; // FPS
    const double FRAME_DURATION = 1000 / FRAMERATE;

    uint8_t MAX_HISTORY_LENGTH = USE_TEACHY_TV ? 2 : 10;
    double SEED_BUMPS[] = {0, 1, -1, 2, -2};

    uint64_t CONTINUE_SCREEN_FRAMES = 200;

    const int64_t FIXED_SEED_OFFSET = -845; // milliseconds. approximate;
    double SEED_CALIBRATION_FRAMES = RNG_CALIBRATION.seed_calibration / FRAME_DURATION;
    double ADVANCES_CALIBRATION = RNG_CALIBRATION.advances_calibration;
    double CONTINUE_SCREEN_ADJUSTMENT = RNG_CALIBRATION.csf_calibration;

    AdvRngSearcher searcher(TARGET_SEED, ADVANCES, AdvRngMethod::Method1);
    AdvPokemonResult target_result = searcher.generate_pokemon();
    env.log("Target IVs:");
    env.log("HP: " + std::to_string(target_result.ivs.hp));
    env.log("Atk: " + std::to_string(target_result.ivs.attack));
    env.log("Def: " + std::to_string(target_result.ivs.defense));
    env.log("SpA: " + std::to_string(target_result.ivs.spatk));
    env.log("SpD: " + std::to_string(target_result.ivs.spdef));
    env.log("Spe: " + std::to_string(target_result.ivs.speed));

    RngAdvanceHistory ADVANCE_HISTORY;
    RngCalibrationHistory CALIBRATION_HISTORY; 
    uint64_t INITIAL_ADVANCES_RADIUS = USE_TEACHY_TV ? 8192 : 1024;
    uint64_t resets = 0;
    bool wildshiny_found = false;

    while (true){
        if (CALIBRATION_HISTORY.results.size() > 0){
            env.log("Checking for nonshiny target hit...");
            if (have_hit_target(env, TARGET_SEED, CALIBRATION_HISTORY.results.back())){
                env.log("Target Hit!");
                stats.nonshiny++;
                break;
            }
            env.log("Missed target.");
        }

        if (resets > MAX_RESETS){
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

        uint64_t advances_radius = INITIAL_ADVANCES_RADIUS;
        for (size_t i=0; i<CALIBRATION_HISTORY.results.size(); i++){
            advances_radius = advances_radius / 2;
            if (advances_radius <= 4){
                advances_radius = 4;
                break;
            }
        }
        env.log("Advances search radius: " + std::to_string(advances_radius));

        if (CALIBRATION_HISTORY.results.size() > 0){
            SEED_CALIBRATION_FRAMES = get_seed_calibration_frames(CALIBRATION_HISTORY, SEED_VALUES, SEED_POSITION);
            ADVANCES_CALIBRATION = get_advances_calibration_frames(CALIBRATION_HISTORY, ADVANCES);
        }

        if (CALIBRATION_HISTORY.results.size() > 0){
            AdvRngState prev_hit = CALIBRATION_HISTORY.results.back();
            double prev_csf_calibration = CALIBRATION_HISTORY.continue_screen_adjustments.back();
            int64_t prev_advance_miss = int64_t(prev_hit.advance) - int64_t(ADVANCES);
            if (prev_advance_miss != 0 && std::abs(prev_advance_miss) < 2){
                env.log("Attempting to correct for off-by-one miss by modifying continue screen frames.");
                if (prev_advance_miss > 0){
                    CONTINUE_SCREEN_ADJUSTMENT = prev_csf_calibration - 0.5;
                }else{
                    CONTINUE_SCREEN_ADJUSTMENT = prev_csf_calibration + 0.5;
                }
                CONTINUE_SCREEN_ADJUSTMENT = fmod(CONTINUE_SCREEN_ADJUSTMENT, 2);
            }else{
                // we're still not that close. Slightly vary the seed to more reliably hone in on advances
                double seed_bump = SEED_BUMPS[ADVANCE_HISTORY.results.size() % 5];
                SEED_CALIBRATION_FRAMES += seed_bump;
            }
        }else{
            double seed_bump = SEED_BUMPS[ADVANCE_HISTORY.results.size() % 5];
            SEED_CALIBRATION_FRAMES += seed_bump;
        }

        double CALIBRATED_ADVANCES = ADVANCES + ADVANCES_CALIBRATION;
        double INGAME_ADVANCES = CALIBRATED_ADVANCES - CONTINUE_SCREEN_FRAMES - CONTINUE_SCREEN_ADJUSTMENT;

        double TEACHY_ADVANCES = 0;
        bool should_use_teachy_tv = USE_TEACHY_TV && (INGAME_ADVANCES > 5000); // don't use Teachy TV for short in-game advance targets
        if (should_use_teachy_tv) {
            TEACHY_ADVANCES = std::floor((INGAME_ADVANCES - 5000) / 313) * 313;
        }

        env.log("Seed calibration (frames): " + std::to_string(SEED_CALIBRATION_FRAMES));
        env.log("Advance calibration (frames / 2): " + std::to_string(ADVANCES_CALIBRATION));
        env.log("Continue screen adjustment (frames): " + std::to_string(CONTINUE_SCREEN_ADJUSTMENT));

        uint64_t CALIBRATED_SEED_DELAY = uint64_t(std::round(SEED_DELAY + FIXED_SEED_OFFSET + FRAME_DURATION * SEED_CALIBRATION_FRAMES));
        uint64_t CONTINUE_SCREEN_DELAY =  uint64_t(std::round(FRAME_DURATION * (CONTINUE_SCREEN_FRAMES + CONTINUE_SCREEN_ADJUSTMENT)));
        uint64_t TEACHY_DELAY = uint64_t(TEACHY_ADVANCES * FRAME_DURATION / 313);
        uint64_t INGAME_DELAY = uint64_t(std::round(FRAME_DURATION * (INGAME_ADVANCES - TEACHY_ADVANCES) / 2)) - (should_use_teachy_tv ? 13700 : 0);

        env.log("Title screen duration: " + std::to_string(CALIBRATED_SEED_DELAY) + "ms");
        env.log("Continue screen duration: " + std::to_string(CONTINUE_SCREEN_DELAY) + "ms");
        if (should_use_teachy_tv){
            env.log("Teachy TV duration: " + std::to_string(TEACHY_DELAY) + "ms");
            env.log("Non-Teachy TV in-game duration: " + std::to_string(INGAME_DELAY) + "ms");
        }else{
            env.log("In-game duration: " + std::to_string(INGAME_DELAY) + "ms");
        }

        env.log("Resetting Game...");
        reset_and_perform_blind_sequence(
            env.console, context, TARGET, 
            SEED_BUTTON, EXTRA_BUTTON, CALIBRATED_SEED_DELAY, 
            CONTINUE_SCREEN_DELAY, TEACHY_DELAY, INGAME_DELAY, 
            false, PROFILE
        );
        stats.resets++; 

        RNG_FILTERS.reset();
        RNG_CALIBRATION.reset();

        bool shiny_found = check_for_shiny(env.console, context, TARGET);

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

        AdvObservedPokemon pokemon = read_summary(env, context);
        AdvRngFilters filters = observation_to_filters(pokemon, BASE_STATS);
        RNG_FILTERS.set(filters);

        std::map<AdvRngState, AdvPokemonResult> search_hits = get_search_results(env.console, searcher, filters, SEED_VALUES, ADVANCES, advances_radius, GENDER_THRESHOLD);
        RNG_CALIBRATION.set(
            SEED_CALIBRATION_FRAMES * FRAME_DURATION,
            CONTINUE_SCREEN_ADJUSTMENT,
            ADVANCES_CALIBRATION - CONTINUE_SCREEN_ADJUSTMENT,
            search_hits
        );        
        bool finished = update_history(env.console, ADVANCE_HISTORY, CALIBRATION_HISTORY, MAX_HISTORY_LENGTH, SEED_CALIBRATION_FRAMES, ADVANCES_CALIBRATION, CONTINUE_SCREEN_ADJUSTMENT, search_hits, 1);
        if (finished || (MAX_RARE_CANDIES == 0)){
            env.log("RNG search finished.");
            continue;
        }

        for (uint64_t i=0; i<MAX_RARE_CANDIES; i++){
            bool failed = use_rare_candy(env, context, pokemon, filters, BASE_STATS, i == 0);
            if (failed){
                stats.errors++;
            }

            search_hits = get_search_results(env.console, searcher, filters, SEED_VALUES, ADVANCES, advances_radius, GENDER_THRESHOLD);
            RNG_CALIBRATION.set(
                SEED_CALIBRATION_FRAMES * FRAME_DURATION,
                CONTINUE_SCREEN_ADJUSTMENT,
                ADVANCES_CALIBRATION - CONTINUE_SCREEN_ADJUSTMENT,
                search_hits
            );

            bool force_finish = failed || (i == (MAX_RARE_CANDIES - 1));
            finished = update_history(
                env.console, ADVANCE_HISTORY, 
                CALIBRATION_HISTORY, MAX_HISTORY_LENGTH, 
                SEED_CALIBRATION_FRAMES, ADVANCES_CALIBRATION, 
                CONTINUE_SCREEN_ADJUSTMENT, search_hits, 
                1, 2, force_finish
            );

            if (finished){
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
