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
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Inference/NintendoSwitch_HomeMenuDetector.h"
#include "NintendoSwitch/Inference/NintendoSwitch_UpdatePopupDetector.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_BattleDialogs.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_SummaryDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_BattlePokemonDetector.h"
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
    , USE_COPYRIGHT_TEXT(
        "<b>Detect Copyright Text:</b><br>Start the seed timer only after detecting the copyright text. Can be helpful if your seeds are inconsistent.",
        LockMode::LOCK_WHILE_RUNNING,
        true // default
    )
    , IGNORE_WILD_SHINIES(
        "<b>Ignore wild shinies</b><br>Do not stop the program when a wild shiny is encountered.",
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
    PA_ADD_OPTION(POSSIBLE_HITS);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(STARTER);
    PA_ADD_OPTION(MAX_RESETS);
    PA_ADD_OPTION(SEED);
    PA_ADD_OPTION(SEED_LIST);
    PA_ADD_OPTION(SEED_BUTTON);
    PA_ADD_OPTION(SEED_DELAY);
    PA_ADD_OPTION(ADVANCES);
    // PA_ADD_OPTION(CONTINUE_SCREEN_FRAMES);
    PA_ADD_OPTION(USE_COPYRIGHT_TEXT);
    PA_ADD_OPTION(PROFILE);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}


namespace {

void check_seed_validity(SingleSwitchProgramEnvironment& env, std::string seed_string){
 static const std::map<char, char> MAP{
        {'1', '1'},
        {'2', '2'},
        {'3', '3'},
        {'4', '4'},
        {'5', '5'},
        {'6', '6'},
        {'7', '7'},
        {'8', '8'},
        {'9', '9'},
        {'0', '0'},
        {'A', 'A'}, {'a', 'A'},
        {'B', 'B'}, {'b', 'B'},
        {'C', 'C'}, {'c', 'C'},
        {'D', 'D'}, {'d', 'D'},
        {'E', 'E'}, {'e', 'E'},
        {'F', 'F'}, {'f', 'F'}
    };

    if (seed_string.size() != 4){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "StarterRng(): Invalid seed length. Seeds should be 4 characters.",
            env.console
        ); 
    }

    for (char ch : seed_string){
        auto iter = MAP.find(ch);
        if (iter == MAP.end()){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "StarterRng(): Invalid seed character. Seeds should be hex strings (valid characters are 0-9 and A-F).",
                env.console
            ); 
        }
    }
}

uint16_t parse_seed(SingleSwitchProgramEnvironment& env, std::string seed_string){
    check_seed_validity(env, seed_string);
    std::istringstream converter(seed_string);
    uint16_t value;
    converter >> std::hex >> value;
    return value;
}

std::vector<uint16_t> parse_seed_list(SingleSwitchProgramEnvironment& env, std::string seed_list_string){
    std::vector<uint16_t> values;
    auto ss = std::stringstream{seed_list_string};
    for (std::string line; std::getline(ss, line, '\n');){
        values.push_back(parse_seed(env, line));
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

} // namespace

bool StarterRng::have_hit_target(SingleSwitchProgramEnvironment& env, const uint32_t TARGET_SEED, AdvRngState& hit){
    return (hit.seed == TARGET_SEED) && (hit.advance == ADVANCES);
}

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


void StarterRng::update_filters(AdvRngFilters& filters, AdvObservedPokemon& pokemon, StatReads& stats, EVs& evyield, BaseStats& BASE_STATS){
    pokemon.level.emplace_back(pokemon.level.back() + 1);
    pokemon.stats.emplace_back(stats);
    pokemon.evs.emplace_back(evyield);

    AdvRngFilters new_filters = observation_to_filters(pokemon, BASE_STATS);
    filters.ivs = new_filters.ivs;
    RNG_FILTERS.set(filters);
}

std::map<AdvRngState, AdvPokemonResult> StarterRng::get_starter_search_results(
    SingleSwitchProgramEnvironment& env,
    AdvRngSearcher& searcher, 
    AdvRngFilters& filters,
    const std::vector<uint16_t>& SEED_VALUES,
    const uint64_t& ADVANCES, 
    uint64_t& advances_radius, 
    AdvObservedPokemon& pokemon
){
    std::map<AdvRngState, AdvPokemonResult> search_hits;
    for (int i=0; i<4; i++){
        uint64_t adv_radius = advances_radius * (uint64_t(1) << i);
        uint64_t min_adv = ADVANCES - std::min(uint64_t(ADVANCES), adv_radius);    
        uint64_t max_adv = ADVANCES + adv_radius;
        search_hits = searcher.search(filters, SEED_VALUES, min_adv, max_adv, 0, 30);
        if (search_hits.size() > 0){
            env.log("Number of search hits: " + std::to_string(search_hits.size()));
            POSSIBLE_HITS.set(search_hits);
            return search_hits;
        }
    }
    env.log("Number of search hits: " + std::to_string(search_hits.size()));
    POSSIBLE_HITS.set(search_hits);
    return search_hits;
}

double StarterRng::get_seed_calibration_frames(
    StarterRngCalibrationHistory& HISTORY, 
    const std::vector<uint16_t>& SEED_VALUES, 
    const int16_t& SEED_POSITION
){
    double sum = 0;
    uint16_t len = 0;
    for (size_t i=0; i<HISTORY.results.size(); i++){
        uint16_t seed = HISTORY.results[i].seed;
        int16_t position = -1;
        for (size_t j=0; j<SEED_VALUES.size(); j++){
            if (seed == SEED_VALUES[j]){
                position = int16_t(j);
                break;
            }
        }
        if (position < 0){
            continue;
        }
        double calibration = HISTORY.seed_calibrations[i];
        double offset = SEED_POSITION - position + calibration;
        sum += offset;
        len++;
    }

    if (len == 0){
        return 0;
    }

    double average_offset = sum / len;
    return average_offset;
}

double StarterRng::get_advances_calibration_frames(StarterRngCalibrationHistory& CALIBRATION_HISTORY, uint64_t ADVANCES){
    double sum = 0;
    uint16_t len = 0;
    for (size_t i=0; i<CALIBRATION_HISTORY.results.size(); i++){
        uint64_t advance = CALIBRATION_HISTORY.results[i].advance;
        double calibration = CALIBRATION_HISTORY.advance_calibrations[i];
        double offset = calibration + ADVANCES - advance;
        sum += offset;
        len++;
    }

    if (len == 0){
        return 0;
    }

    double average_offset = sum / len;
    return average_offset;
}

bool StarterRng::update_history(
    SingleSwitchProgramEnvironment& env,
    StarterRngAdvanceHistory& ADVANCE_HISTORY,
    StarterRngCalibrationHistory& CALIBRATION_HISTORY, 
    const uint16_t& MAX_HISTORY_LENGTH,
    double& SEED_CALIBRATION_FRAMES,
    double& ADVANCES_CALIBRATION,
    double& CONTINUE_SCREEN_ADJUSTMENT,
    std::map<AdvRngState, AdvPokemonResult>& search_hits,
    bool force_finish
){
    const int MAX_ADVANCE_POSSIBILITIES = 5;
    const uint32_t ADVANCE_RADIUS = 2;

    if (search_hits.size() == 0){
        env.log("No matches found.");
        return true;
    }

    if (!force_finish && search_hits.size() > MAX_ADVANCE_POSSIBILITIES){
        return false;
    }

    if (search_hits.size() == 1){
        env.log("Updating calibrations...");
        CALIBRATION_HISTORY.seed_calibrations.emplace_back(SEED_CALIBRATION_FRAMES);
        CALIBRATION_HISTORY.advance_calibrations.emplace_back(ADVANCES_CALIBRATION);
        CALIBRATION_HISTORY.continue_screen_adjustments.emplace_back(CONTINUE_SCREEN_ADJUSTMENT);
        CALIBRATION_HISTORY.results.emplace_back(search_hits.begin()->first);
        if (CALIBRATION_HISTORY.results.size() > MAX_HISTORY_LENGTH){
            CALIBRATION_HISTORY.seed_calibrations.erase(CALIBRATION_HISTORY.seed_calibrations.begin());
            CALIBRATION_HISTORY.advance_calibrations.erase(CALIBRATION_HISTORY.advance_calibrations.begin());
            CALIBRATION_HISTORY.continue_screen_adjustments.erase(CALIBRATION_HISTORY.continue_screen_adjustments.begin());
            CALIBRATION_HISTORY.results.erase(CALIBRATION_HISTORY.results.begin());
        }
        ADVANCE_HISTORY.results.clear();
        ADVANCE_HISTORY.seed_calibrations.clear();
        return true;
    }
    
    std::vector<uint64_t> advances;
    std::vector<AdvRngState> hits;
    for(std::map<AdvRngState,AdvPokemonResult>::iterator it=search_hits.begin(); it!=search_hits.end(); ++it) {
        advances.emplace_back(it->first.advance);
        hits.emplace_back(it->first);
    }
    
    // get unique advances
    std::sort(advances.begin(), advances.end());
    std::vector<uint64_t>::iterator iter;
    iter = std::unique(advances.begin(), advances.begin() + advances.size());
    advances.resize(std::distance(advances.begin(), iter));

    ADVANCE_HISTORY.seed_calibrations.emplace_back(SEED_CALIBRATION_FRAMES);
    ADVANCE_HISTORY.results.emplace_back(hits);
    
    // check advance history for repeated values
    std::vector<uint64_t> counts;
    uint64_t best = 0;
    uint64_t mode = 0;
    bool tie = false;
    for (uint64_t& adv : advances){
        uint64_t count = 0;
        for (auto& res : ADVANCE_HISTORY.results){
            for (auto& state : res){
                if (std::abs(int64_t(state.advance) - int64_t(adv)) <= ADVANCE_RADIUS){
                    count++;
                    break; // only count one possible hit from each attempt
                }
            }
        }
        if (count > best){
            mode = adv;
            best = count;
            tie = false;
        }else if (count == best){
            tie = true;
        }
    }

    if (tie){
        env.log("More than 1 possible advances value hit.");
        return true;
    }


    // add the closest possibility to the advances mode for each attempt to the calibration history
    env.log("Inferred hits from previous " + std::to_string(ADVANCE_HISTORY.results.size()) + " attempts: ");
    for (size_t i=0; i<ADVANCE_HISTORY.results.size(); i++){
        auto& res = ADVANCE_HISTORY.results[i];
        auto& seed_calibration = ADVANCE_HISTORY.seed_calibrations[i];
        AdvRngState most_likely_hit;
        int64_t lowest_dist = INTMAX_MAX;
        for (auto& state : res){
            int64_t state_dist = std::abs(int64_t(state.advance) - int64_t(mode));
            if (state_dist < lowest_dist){
                most_likely_hit = state;
                lowest_dist = state_dist;
            }
        }
        CALIBRATION_HISTORY.seed_calibrations.emplace_back(seed_calibration);
        CALIBRATION_HISTORY.advance_calibrations.emplace_back(ADVANCES_CALIBRATION);
        CALIBRATION_HISTORY.continue_screen_adjustments.emplace_back(CONTINUE_SCREEN_ADJUSTMENT);
        CALIBRATION_HISTORY.results.emplace_back(most_likely_hit);
        env.log("   " + std::to_string(most_likely_hit.seed) + " / " + std::to_string(most_likely_hit.advance));
    }


    while (CALIBRATION_HISTORY.results.size() > MAX_HISTORY_LENGTH){
        CALIBRATION_HISTORY.seed_calibrations.erase(CALIBRATION_HISTORY.seed_calibrations.begin());
        CALIBRATION_HISTORY.advance_calibrations.erase(CALIBRATION_HISTORY.advance_calibrations.begin());
        CALIBRATION_HISTORY.continue_screen_adjustments.erase(CALIBRATION_HISTORY.continue_screen_adjustments.begin());
        CALIBRATION_HISTORY.results.erase(CALIBRATION_HISTORY.results.begin());
    }

    ADVANCE_HISTORY.results.clear();
    ADVANCE_HISTORY.seed_calibrations.clear();

    return true;
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

    return (ret < 0);
}

bool StarterRng::auto_battle_rival(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    AdvObservedPokemon& pokemon,
    AdvRngFilters& filters,
    BaseStats& BASE_STATS
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
        env.log("auto_battle_rival(): failed to detect the battle menu.");
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
        env.log("auto_battle_rival(): failed to detect the battle menu.");
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
        env.log("auto_battle_rival(): no fainting detected with 5 minutes.");
        return true;
    }

    // slowly advance dialog until level-up stats are visible
    BattleLevelUpWatcher level_up_stats(COLOR_RED, BattleLevelUpDialog::stats);
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
        env.log("walk_to_route1_from_lab(): failed to exit lab.");
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
        env.log("walk_to_route1_from_home(): failed to exit the house.");
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
    BaseStats& BASE_STATS
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
            env.log("autolevel_on_route1(): failed to trigger encounter.");
            return -1;
        }

        if (ret == 1){ // shiny
            return 1;
        }

        // auto battle
        BattleResult ret2 = spam_first_move(env.console, context);

        BattleLevelUpWatcher level_up(COLOR_RED, BattleLevelUpDialog::stats);
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
    POSSIBLE_HITS.reset();

    const uint16_t TARGET_SEED = parse_seed(env, SEED);
    const std::vector<uint16_t> SEED_VALUES = parse_seed_list(env, SEED_LIST);
    const int16_t SEED_POSITION = seed_position_in_list(TARGET_SEED, SEED_VALUES);

    if (SEED_POSITION == -1){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "StarterRng(): Target Seed is missing from the list of nearby seeds.",
            env.console
        ); 
    }

    env.log("Target Seed Value: " + std::to_string(TARGET_SEED));

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

    uint8_t MAX_HISTORY_LENGTH = 10;
    double SEED_BUMPS[] = {0, 1, -1, 2, -2};

    uint64_t CONTINUE_SCREEN_FRAMES = 200;

    const uint64_t FIXED_SEED_OFFSET = USE_COPYRIGHT_TEXT ? -2140 : -845; // milliseconds. approximate;
    double SEED_CALIBRATION_FRAMES = 0;
    double ADVANCES_CALIBRATION = 0;
    double CONTINUE_SCREEN_ADJUSTMENT = 0;

    AdvRngSearcher searcher(TARGET_SEED, ADVANCES, AdvRngMethod::Method1);
    AdvPokemonResult target_result = searcher.generate_pokemon();
    env.log("Target IVs:");
    env.log("HP: " + std::to_string(target_result.ivs.hp));
    env.log("Atk: " + std::to_string(target_result.ivs.attack));
    env.log("Def: " + std::to_string(target_result.ivs.defense));
    env.log("SpA: " + std::to_string(target_result.ivs.spatk));
    env.log("SpD: " + std::to_string(target_result.ivs.spdef));
    env.log("Spe: " + std::to_string(target_result.ivs.speed));

    StarterRngAdvanceHistory ADVANCE_HISTORY;
    StarterRngCalibrationHistory CALIBRATION_HISTORY; 
    uint64_t INITIAL_ADVANCES_RADIUS = 1024;
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

        SEED_CALIBRATION_FRAMES = get_seed_calibration_frames(CALIBRATION_HISTORY, SEED_VALUES, SEED_POSITION);
        ADVANCES_CALIBRATION = get_advances_calibration_frames(CALIBRATION_HISTORY, ADVANCES);

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

        env.log("Seed calibration (frames): " + std::to_string(SEED_CALIBRATION_FRAMES));
        env.log("Advance calibration (frames / 2): " + std::to_string(ADVANCES_CALIBRATION));
        env.log("Continue screen adjustment (frames): " + std::to_string(CONTINUE_SCREEN_ADJUSTMENT));

        uint64_t CALIBRATED_SEED_DELAY = uint64_t(std::round(SEED_DELAY + FIXED_SEED_OFFSET + FRAME_DURATION * SEED_CALIBRATION_FRAMES));
        uint64_t CONTINUE_SCREEN_DELAY =  uint64_t(std::round(FRAME_DURATION * (CONTINUE_SCREEN_FRAMES + CONTINUE_SCREEN_ADJUSTMENT)));
        uint64_t INGAME_DELAY =  uint64_t(std::round(FRAME_DURATION * INGAME_ADVANCES / 2));

        env.log("Title screen duration: " + std::to_string(CALIBRATED_SEED_DELAY) + "ms");
        env.log("Continue screen duration: " + std::to_string(CONTINUE_SCREEN_DELAY) + "ms");
        env.log("In-game duration: " + std::to_string(INGAME_DELAY) + "ms");

        env.log("Resetting Game...");
        if (USE_COPYRIGHT_TEXT){
            reset_and_detect_copyright_text(env.console, context, PROFILE);
            env.log("Starting blind button presses.");
            perform_blind_sequence(context, PokemonFRLG_RngTarget::starters, SEED_BUTTON, CALIBRATED_SEED_DELAY, CONTINUE_SCREEN_DELAY, 0, INGAME_DELAY, false);
        }else{
            reset_and_perform_blind_sequence(
                env.console, context, PokemonFRLG_RngTarget::starters, SEED_BUTTON, CALIBRATED_SEED_DELAY, CONTINUE_SCREEN_DELAY, 0, INGAME_DELAY, false, PROFILE);
        }
        stats.resets++; 

        RNG_FILTERS.reset();
        POSSIBLE_HITS.reset();

        bool shiny_found = check_for_shiny(env.console, context, PokemonFRLG_RngTarget::starters);

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

        std::map<AdvRngState, AdvPokemonResult> search_hits = get_starter_search_results(env, searcher, filters, SEED_VALUES, ADVANCES, advances_radius, pokemon);
        bool finished = update_history(env, ADVANCE_HISTORY, CALIBRATION_HISTORY, MAX_HISTORY_LENGTH, SEED_CALIBRATION_FRAMES, ADVANCES_CALIBRATION, CONTINUE_SCREEN_ADJUSTMENT, search_hits);
        if (finished){
            env.log("RNG search finished.");
            continue;
        }

        bool failed = walk_to_rival_battle(env, context);
        if (failed){
            stats.errors++;
            env.log("Failed to initiate rival battle.");
            continue; // reset game
        }

        failed = auto_battle_rival(env, context, pokemon, filters, BASE_STATS);
        if (failed){
            stats.errors++;
            continue; // reset game
        }
        if (pokemon.level.size() > 1){
            searcher.refine_search(search_hits, filters, 0, 30);
            POSSIBLE_HITS.set(search_hits);
            env.log("Number of search hits: " + std::to_string(search_hits.size()));
            finished = update_history(env, ADVANCE_HISTORY, CALIBRATION_HISTORY, MAX_HISTORY_LENGTH, SEED_CALIBRATION_FRAMES, ADVANCES_CALIBRATION, CONTINUE_SCREEN_ADJUSTMENT, search_hits);
            if (finished){
                env.log("RNG search finished.");
                continue;
            }
        }

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
                update_history(env, ADVANCE_HISTORY, CALIBRATION_HISTORY, MAX_HISTORY_LENGTH, SEED_CALIBRATION_FRAMES, ADVANCES_CALIBRATION, CONTINUE_SCREEN_ADJUSTMENT, search_hits, true);
                break;
            }

            env.log("Level: " + std::to_string(4 + pokemon.level.size()));
            env.log("Speed EVs: " + std::to_string(pokemon.evs.back().speed));

            int ret2 = autolevel_on_route1(env, context, pokemon, filters, BASE_STATS);
            if (ret2 < 0){
                env.log("Error encountered while auto-leveling.");
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
            
            if (pokemon.level.size() > num_levels){
                num_levels = pokemon.level.size();
                searcher.refine_search(search_hits, filters, 0, 30);
                POSSIBLE_HITS.set(search_hits);
                env.log("Number of search hits: " + std::to_string(search_hits.size()));
                finished = update_history(env, ADVANCE_HISTORY, CALIBRATION_HISTORY, MAX_HISTORY_LENGTH, SEED_CALIBRATION_FRAMES, ADVANCES_CALIBRATION, CONTINUE_SCREEN_ADJUSTMENT, search_hits);
                if (finished){
                    env.log("RNG search finished.");
                    break;
                }
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
