/*  Starter RNG
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
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Pokemon_StatsCalculation.h"
#include "Pokemon/Pokemon_AdvRng.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Inference/NintendoSwitch_HomeMenuDetector.h"
#include "NintendoSwitch/Inference/NintendoSwitch_UpdatePopupDetector.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_BattleDialogs.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_SummaryDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_BattleLevelUpReader.h"
#include "PokemonFRLG/Inference/PokemonFRLG_ShinySymbolDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_StatsReader.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_BlindNavigation.h"
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
        "Automatically calibrate timings to hit a specific RNG target.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct StarterRng_Descriptor::Stats : public StatsTracker{
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
std::unique_ptr<StatsTracker> StarterRng_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

StarterRng::StarterRng()
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
    , STARTER(
        "<b>Target:</b><br>",
        {
            {Starter::bulbasaur, "bulbasaur", "Bulbasaur"},
            {Starter::squirtle, "squirtle", "Squirtle"},
            {Starter::charmander, "charmander", "Charmander"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Starter::bulbasaur
    )    
    , MAX_RESETS(
        "<b>Max Resets:</b><br>",
        LockMode::UNLOCK_WHILE_RUNNING,
        50, 0 // default, min
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
    , SEED_DELAY(
        "<b>Seed Delay Time (ms):</b><br>The delay between starting the game and advancing past the title screen. Set this to match your target seed.",
        LockMode::LOCK_WHILE_RUNNING,
        35000, 28000 // default, min
    )
    , ADVANCES(
        "<b>Advances:</b><br>The total number of RNG advances for your target.<br>This should be the combined amount of continue screen and in-game advances.",
        LockMode::LOCK_WHILE_RUNNING,
        10000, 600 // default, min
    )
    , CONTINUE_SCREEN_FRAMES(
        "<b>Continue Screen Frames:</b><br>The number of RNG advances to pass on the continue screen.<br>This should be less than the total number of advances above.",
        LockMode::LOCK_WHILE_RUNNING,
        1000, 192 // default, min
    )
    , USE_COPYRIGHT_TEXT(
        "<b>Detect Copyright Text:</b><br>Start the seed timer only after detecting the copyright text. Can be helpful if your seeds are inconsistent.",
        LockMode::LOCK_WHILE_RUNNING,
        true // default
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
    PA_ADD_OPTION(POSSIBLE_HITS);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(STARTER);
    PA_ADD_OPTION(MAX_RESETS);
    PA_ADD_OPTION(SEED);
    PA_ADD_OPTION(SEED_LIST);
    PA_ADD_OPTION(SEED_BUTTON);
    PA_ADD_OPTION(SEED_DELAY);
    PA_ADD_OPTION(ADVANCES);
    PA_ADD_OPTION(CONTINUE_SCREEN_FRAMES);
    PA_ADD_OPTION(USE_COPYRIGHT_TEXT);
    PA_ADD_OPTION(PROFILE);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}


namespace {

uint16_t parse_seed(std::string seed_string){
    std::istringstream converter(seed_string);
    uint16_t value;
    converter >> std::hex >> value;
    return value;
}

std::vector<uint16_t> parse_seed_list(std::string seed_list_string){
    std::vector<std::string> seed_strings = {};
    auto ss = std::stringstream{seed_list_string};
    for (std::string line; std::getline(ss, line, '\n');){
        seed_strings.push_back(line);
    }

    std::vector<uint16_t> values;
    for (size_t i=0; i<seed_strings.size(); i++){
        values.push_back(parse_seed(seed_strings[i]));
    }
    return values;    
}

int16_t seed_position_in_list(uint16_t seed, std::vector<uint16_t> list){
    for (size_t i=0; i<list.size(); i++){
        if (seed == list[i]) {
            return int16_t(i);
        }
    }
    return -1;
}

AdvNature string_to_nature(std::string nature_string){
    if (nature_string == "Hardy")   return AdvNature::Hardy;
    if (nature_string == "Lonely")  return AdvNature::Lonely;    
    if (nature_string == "Brave")   return AdvNature::Brave;  
    if (nature_string == "Adamant") return AdvNature::Adamant;  
    if (nature_string == "Naughty") return AdvNature::Naughty;  
    if (nature_string == "Bold")    return AdvNature::Bold;  
    if (nature_string == "Docile")  return AdvNature::Docile;  
    if (nature_string == "Relaxed") return AdvNature::Relaxed;  
    if (nature_string == "Impish")  return AdvNature::Impish;  
    if (nature_string == "Lax")     return AdvNature::Lax;  
    if (nature_string == "Timid")   return AdvNature::Timid;  
    if (nature_string == "Hasty")   return AdvNature::Hasty;  
    if (nature_string == "Serious") return AdvNature::Serious;  
    if (nature_string == "Jolly")   return AdvNature::Jolly;  
    if (nature_string == "Naive")   return AdvNature::Naive;  
    if (nature_string == "Modest")  return AdvNature::Modest;  
    if (nature_string == "Mild")    return AdvNature::Mild;  
    if (nature_string == "Quiet")   return AdvNature::Quiet;  
    if (nature_string == "Bashful") return AdvNature::Bashful;  
    if (nature_string == "Rash")    return AdvNature::Rash;  
    if (nature_string == "Calm")    return AdvNature::Calm;  
    if (nature_string == "Gentle")  return AdvNature::Gentle;  
    if (nature_string == "Sassy")   return AdvNature::Sassy;  
    if (nature_string == "Careful") return AdvNature::Careful;  
    if (nature_string == "Quirky")  return AdvNature::Quirky;  
    return AdvNature::Any;
}

double get_seed_calibration_frames(std::vector<AdvRngState>& HISTORY, const std::vector<uint16_t>& SEED_VALUES, const int16_t& SEED_POSITION){
    double sum = 0;
    uint16_t len = 0;
    for (size_t i=0; i<HISTORY.size(); i++){
        uint16_t seed = HISTORY[i].seed;
        int16_t position = -1;
        for (size_t j=0; j<SEED_VALUES.size(); i++){
            if (seed == SEED_VALUES[j]){
                position = int16_t(j);
                break;
            }
        }
        if (position < 0){
            continue;
        }
        int16_t offset = SEED_POSITION - position;
        sum += offset;
        len++;
    }

    if (len == 0){
        return 0;
    }

    double average_offset = sum / len;
    return average_offset;
}

double get_advances_calibration_frames(std::vector<AdvRngState>& HISTORY, uint64_t ADVANCES){
    double sum = 0;
    uint16_t len = 0;
    for (size_t i=0; i<HISTORY.size(); i++){
        uint64_t advance = HISTORY[i].advance;
        int64_t offset = ADVANCES - advance;
        sum += offset;
    }

    if (len == 0){
        return 0;
    }

    double average_offset = sum / len;
    return average_offset;
}

} // namespace


AdvObservedPokemon StarterRng::read_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
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


void StarterRng::walk_to_rival_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context, Starter STARTER){

}

void StarterRng::auto_battle_rival(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AdvObservedPokemon& pokemon){
    // Pokemon::EVs evyield = {0, 0, 0, 0, 0, 0};
    // Pokemon::StatReads stats;



}



void StarterRng::walk_to_route1_from_lab(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

}

void StarterRng::walk_home_from_route1(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

}

void StarterRng::heal_at_home(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

}

void StarterRng::walk_to_route1_from_home(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

}

bool StarterRng::autolevel_on_route1(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AdvObservedPokemon& pokemon){
    Pokemon::EVs evyield = {0, 0, 0, 0, 0, 0};
    Pokemon::StatReads stats;

    bool leftright = false;
    pbf_move_left_joystick(context, {-1, 0}, 100ms, 0ms);
    context.wait_for_all_requests();

    while (true){
        // trigger encounter
        int ret = grass_spin(env.console, context, leftright);
        if (ret < 0){
            env.log("autolevel_on_route1(): failed to trigger encounter.");
            return false;
        }

        // auto battle
        BattleResult ret2 = spam_first_move(env.console, context);

        BattleLevelUpWatcher level_up(COLOR_RED, BattleLevelUpDialog::stats);
        BlackScreenWatcher black_screen(COLOR_RED);
        VideoSnapshot screen;
        int ret3;
        BattleLevelUpReader reader;          

        switch (ret2){
        case BattleResult::opponentfainted:
            evyield.speed++; // always rattata or pidgey
            leftright = !leftright;

            context.wait_for_all_requests();
            ret3 = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context) {
                    for (int i=0; i<5; i++){
                        pbf_press_button(context, BUTTON_B, 200ms, 2800ms);
                    }
                },
                { level_up, black_screen }
            );

            switch (ret3){
            case 0:
                screen = env.console.video().snapshot();      
                stats = reader.read_stats(env.logger(), screen);
            case -1:
                exit_wild_battle(env.console, context, false, true);
            default:
                pbf_wait(context, 1000ms);
                context.wait_for_all_requests();
            }

            level_up_observed_pokemon(pokemon, stats, evyield);
            return true;
        case BattleResult::playerfainted:
            walk_to_route1_from_home(env, context);
            pbf_move_left_joystick(context, {-1, 0}, 100ms, 0ms);
            context.wait_for_all_requests();
            continue;
        case BattleResult::outofpp:
            walk_home_from_route1(env, context);
            heal_at_home(env, context);
            walk_to_route1_from_home(env, context);
            pbf_move_left_joystick(context, {-1, 0}, 100ms, 0ms);
            context.wait_for_all_requests();
        case BattleResult::unknown:
        default:
            return false;
        }
    }
}

void StarterRng::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    /*
    * Settings: Text Speed fast
    */

    StarterRng_Descriptor::Stats& stats = env.current_stats<StarterRng_Descriptor::Stats>();

    home_black_border_check(env.console, context);


    const uint16_t TARGET_SEED = parse_seed(SEED);
    const std::vector<uint16_t> SEED_VALUES = parse_seed_list(SEED_LIST);
    const int16_t SEED_POSITION = seed_position_in_list(TARGET_SEED, SEED_VALUES);

    if (SEED_POSITION == -1){
        // error
    }

    uint64_t advances_radius = 1000;
    const uint64_t MAX_ADVANCES = ADVANCES + advances_radius;
    const uint64_t MIN_ADVANCES = ADVANCES - std::min(uint64_t(ADVANCES), advances_radius);

    env.log("Target Seed Value: " + std::to_string(TARGET_SEED));
    env.log("Min Advances: " + std::to_string(MIN_ADVANCES));   
    env.log("Max Advances: " + std::to_string(MAX_ADVANCES));

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

    const double FRAMERATE = 59.999977; // FPS
    const double FRAME_DURATION = 1000 / FRAMERATE;

    uint64_t CONTINUE_SCREEN_FRAMES = 500;

    const uint64_t FIXED_SEED_OFFSET = USE_COPYRIGHT_TEXT ? -2140 : -845; // milliseconds. approximate;
    double SEED_CALIBRATION = 0;
    double ADVANCES_CALIBRATION = 0;

    AdvRng rng(TARGET_SEED, ADVANCES, AdvRngMethod::Method1);
    AdvPokemonResult target_result = rng.generate_pokemon();
    env.log("Target IVs:");
    env.log("HP: " + std::to_string(target_result.ivs.hp));
    env.log("Atk: " + std::to_string(target_result.ivs.attack));
    env.log("Def: " + std::to_string(target_result.ivs.defense));
    env.log("SpA: " + std::to_string(target_result.ivs.spatk));
    env.log("SpD: " + std::to_string(target_result.ivs.spdef));
    env.log("Spe: " + std::to_string(target_result.ivs.speed));
    env.log("Target PID: " + std::to_string(target_result.pid));

    std::vector<AdvRngState> HISTORY = {}; 
    uint64_t resets = 0;

    while (true){

        if (resets > MAX_RESETS){

            break;
        }

        SEED_CALIBRATION = FRAME_DURATION * get_seed_calibration_frames(HISTORY, SEED_VALUES, SEED_POSITION);
        ADVANCES_CALIBRATION = get_advances_calibration_frames(HISTORY, ADVANCES);

        double CALIBRATED_ADVANCES = ADVANCES + ADVANCES_CALIBRATION;
        double INGAME_ADVANCES = CALIBRATED_ADVANCES - CONTINUE_SCREEN_FRAMES;

        uint64_t CALIBRATED_SEED_DELAY = uint64_t(std::round(SEED_DELAY + FIXED_SEED_OFFSET + SEED_CALIBRATION));
        uint64_t CONTINUE_SCREEN_DELAY =  uint64_t(std::round(FRAME_DURATION * CONTINUE_SCREEN_FRAMES));
        uint64_t INGAME_DELAY =  uint64_t(std::round(FRAME_DURATION * INGAME_ADVANCES));

        if (USE_COPYRIGHT_TEXT){
            reset_and_detect_copyright_text(env.console, context, PROFILE);
            perform_blind_sequence(context, PokemonFRLG_RngTarget::starters, SEED_BUTTON, CALIBRATED_SEED_DELAY, CONTINUE_SCREEN_DELAY, 0, INGAME_DELAY, false);
        }else{
            reset_and_perform_blind_sequence(
                env.console, context, PokemonFRLG_RngTarget::starters, SEED_BUTTON, CALIBRATED_SEED_DELAY, CONTINUE_SCREEN_DELAY, 0, INGAME_DELAY, false, PROFILE);
        }

        bool shiny_found = check_for_shiny(env.console, context, PokemonFRLG_RngTarget::starters);

        if (shiny_found){
            // handle
        }

        AdvObservedPokemon pokemon = read_summary(env, context);
        AdvRngFilters filters = observation_to_filter(pokemon, BASE_STATS);
        RNG_FILTERS.set(filters);

        std::map<AdvRngState, AdvPokemonResult> search_hits = rng.search(filters, SEED_VALUES, MIN_ADVANCES, MAX_ADVANCES, 0, 30);
        env.log("Number of search hits: " + std::to_string(search_hits.size()));
        POSSIBLE_HITS.set(search_hits);

        // if (search_hits.size() == 0){
        //     env.log("No matches found. Resetting...");
        // }else if (search_hits.size() == 1){

        // }

        // walk_to_rival_battle(env, context, STARTER);
        // auto_battle_rival(env, context, pokemon);

        // walk_to_route1_from_lab(env, context);

        stats.resets++;
        break;
    }


}



}
}
}
