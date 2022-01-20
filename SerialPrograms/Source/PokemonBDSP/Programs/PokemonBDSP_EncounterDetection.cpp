/*  Encounter Detection
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "PokemonBDSP_EncounterDetection.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


StandardEncounterDetection::StandardEncounterDetection(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    Language language,
    const EncounterFilterOption& filter,
    const DoublesShinyDetection& shininess,
    std::chrono::milliseconds read_name_delay
)
    : m_env(env)
    , m_console(console)
    , m_language(language)
    , m_filter(filter)
    , m_shininess(shininess)
    , m_read_name_delay(read_name_delay)
    , m_double_battle(false)
{
    InferenceBoxScope left_mon_white(console, {0.685, 0.065, 0.025, 0.040});
    InferenceBoxScope left_mon_hp(console, {0.500, 0.120, 0.18, 0.005});
    InferenceBoxScope left_name(console, {0.467, 0.06, 0.16, 0.050});
    InferenceBoxScope right_name(console, {0.740, 0.06, 0.16, 0.050});

    console.botbase().wait_for_all_requests();
    env.wait_for(std::chrono::milliseconds(100));
    QImage screen = console.video().snapshot();

    //  Check if it's a double battle.
    do{
        if (!is_white(extract_box(screen, left_mon_white))){
            break;
        }
        ImageStats stats_hp = image_stats(extract_box(screen, left_mon_hp));
//        cout << stats_hp.average << stats_hp.stddev << endl;
        if (!is_solid(stats_hp, {0.27731, 0.461346, 0.261344}, 0.1, 50)){
            break;
        }
        m_double_battle = true;
    }while (false);

    m_pokemon_left.exists = m_double_battle;
    m_pokemon_right.exists = true;

    //  Read the names.
    if (m_language != Language::None){
        if (m_double_battle){
            m_pokemon_left.detection_enabled = true;
            m_pokemon_left.slugs = read_name(screen, left_name);
        }
        m_pokemon_right.detection_enabled = true;
        m_pokemon_right.slugs = read_name(screen, right_name);
    }

    do{
        if (!is_shiny(m_shininess.shiny_type)){
            m_shininess_left = m_shininess.shiny_type;
            break;
        }
        if (!m_double_battle){
            m_shininess_left = ShinyType::NOT_SHINY;
            break;
        }
        m_shininess_left = m_shininess.left_is_shiny ? ShinyType::UNKNOWN_SHINY : ShinyType::NOT_SHINY;
    }while (false);
    do{
        if (!is_shiny(m_shininess.shiny_type)){
            m_shininess_right = m_shininess.shiny_type;
            break;
        }
        if (!m_double_battle){
            m_shininess_right = m_shininess.shiny_type;
            break;
        }
        m_shininess_right = m_shininess.right_is_shiny ? ShinyType::UNKNOWN_SHINY : ShinyType::NOT_SHINY;
    }while (false);
}
const PokemonDetection& StandardEncounterDetection::pokemon_left() const{
    return m_pokemon_left;
}
const PokemonDetection& StandardEncounterDetection::pokemon_right() const{
    return m_pokemon_right;
}
bool StandardEncounterDetection::has_shiny() const{
    switch (m_shininess.shiny_type){
    case ShinyType::UNKNOWN:
        return false;
    case ShinyType::NOT_SHINY:
        return false;
    case ShinyType::STAR_SHINY:
        return true;
    case ShinyType::SQUARE_SHINY:
        return true;
    case ShinyType::UNKNOWN_SHINY:
        return true;
    }
    return false;
}

std::set<std::string> StandardEncounterDetection::read_name(const QImage& screen, const ImageFloatBox& box){
    QImage image = extract_box(screen, box);

    std::set<std::string> ret;

    OCR::StringMatchResult result = PokemonNameReader::instance().read_substring(m_console, m_language, image);
    if (result.results.empty()){
        dump_image(
            m_console, ProgramInfo(),
            QString::fromStdString("StandardEncounterDetection-NameOCR-" + language_data(m_language).code),
            screen
        );
    }else{
        for (const auto& item : result.results){
            ret.insert(item.second.token);
        }
    }
    return ret;
}




bool filter_match(ShinyType detection, ShinyFilter filter){
    if (detection == ShinyType::UNKNOWN){
        return false;
    }

    switch (filter){
    case ShinyFilter::ANYTHING:
        return true;
    case ShinyFilter::NOT_SHINY:
        return detection == ShinyType::NOT_SHINY;
    case ShinyFilter::SHINY:
        return detection != ShinyType::NOT_SHINY;
    case ShinyFilter::NOTHING:
        return false;
    }

    return false;
}
void StandardEncounterDetection::run_overrides(
    std::vector<std::pair<EncounterAction, std::string>>& actions,
    const std::vector<EncounterFilterOverride>& overrides,
    const PokemonDetection& pokemon, bool side_shiny
) const{
    if (!pokemon.exists || !pokemon.detection_enabled){
        return;
    }
    for (const EncounterFilterOverride& override : overrides){
        //  Not a token match.
        if (pokemon.slugs.find(override.pokemon_slug) == pokemon.slugs.end()){
            continue;
        }

        ShinyType shiny = m_shininess.shiny_type;
        switch (shiny){
        case ShinyType::UNKNOWN:
        case ShinyType::NOT_SHINY:
            break;
        case ShinyType::UNKNOWN_SHINY:
        case ShinyType::STAR_SHINY:
        case ShinyType::SQUARE_SHINY:
            if (m_shininess.left_is_shiny || m_shininess.right_is_shiny){
                shiny = side_shiny ? ShinyType::UNKNOWN_SHINY : ShinyType::NOT_SHINY;
            }else{
                shiny = ShinyType::UNKNOWN;
            }
            break;
        }

        //  Matched the filter.
        if (filter_match(shiny, override.shininess)){
            actions.emplace_back(override.action, override.pokeball_slug);
        }
    }
}


std::pair<EncounterAction, std::string> StandardEncounterDetection::get_action(){
    if (m_shininess.shiny_type == ShinyType::UNKNOWN){
        return {EncounterAction::RunAway, ""};
    }

    std::pair<EncounterAction, std::string> default_action;
    default_action.first = filter_match(m_shininess.shiny_type, m_filter.shiny_filter())
        ? EncounterAction::StopProgram
        : EncounterAction::RunAway;

    const std::vector<EncounterFilterOverride>& overrides = m_filter.overrides();
    if (overrides.empty()){
        return default_action;
    }

    if (m_language == Language::None){
        return default_action;
    }

    std::vector<std::pair<EncounterAction, std::string>> actions;
    run_overrides(actions, overrides, m_pokemon_left, m_shininess.left_is_shiny);
    run_overrides(actions, overrides, m_pokemon_right, m_shininess.right_is_shiny);

    //  No actions matched. Use the default.
    if (actions.empty()){
        return default_action;
    }

    //  Look for conflicts. If any, return stop program.
    bool auto_catch = false;
    for (const std::pair<EncounterAction, std::string>& action : actions){
        if (action.first == EncounterAction::ThrowBalls || action.first == EncounterAction::ThrowBallsAndSave){
            auto_catch = true;
        }
        if (action.first != actions[0].first || action.second != actions[0].second){
            return {EncounterAction::StopProgram, ""};
        }
    }

    //  Double battle and someone is set to auto-catch.
    if (actions.size() > 1 && auto_catch){
        return {EncounterAction::StopProgram, ""};
    }

    //  Otherwise, return the matching action.
    return std::move(actions[0]);
}







}
}
}
