/*  Encounter Detection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonBDSP_EncounterDetection.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


StandardEncounterDetection::StandardEncounterDetection(
    VideoStream& stream, ProControllerContext& context,
    Language language,
    const EncounterFilterOption2& filter,
    const DoublesShinyDetection& shininess,
    std::chrono::milliseconds read_name_delay
)
    : m_stream(stream)
    , m_language(language)
    , m_filter(filter)
    , m_shininess(shininess)
    , m_double_battle(false)
{
//    InferenceBoxScope left_mon_white(stream, {0.685, 0.065, 0.025, 0.040});
    OverlayBoxScope left_mon_white(stream.overlay(), {0.708, 0.070, 0.005, 0.028});
    OverlayBoxScope left_mon_hp(stream.overlay(), {0.500, 0.120, 0.18, 0.005});
    OverlayBoxScope left_name(stream.overlay(), {0.467, 0.06, 0.16, 0.050});
    OverlayBoxScope right_name(stream.overlay(), {0.740, 0.06, 0.16, 0.050});

    context.wait_for_all_requests();
    context.wait_for(std::chrono::milliseconds(100));
    VideoSnapshot screen = stream.video().snapshot();

    //  Check if it's a double battle.
    do{
        if (!is_white(extract_box_reference(screen, left_mon_white))){
            break;
        }
        ImageStats stats_hp = image_stats(extract_box_reference(screen, left_mon_hp));
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

    //  Not a double battle. Pass overall shiny detection to right side.
    if (!m_double_battle){
        m_shininess_left = ShinyType::NOT_SHINY;
        m_shininess_right = m_shininess.shiny_type;
        return;
    }

    //  Not shiny. Pass shiny detection to both sides.
    bool overall_shiny = is_likely_shiny(m_shininess.shiny_type);
    if (!overall_shiny){
        m_shininess_left = m_shininess.shiny_type;
        m_shininess_right = m_shininess.shiny_type;
        return;
    }

    //  Shiny. But neither side takes ownership. Mark both as unknown.
    if (!m_shininess.left_is_shiny && !m_shininess.right_is_shiny){
        m_shininess_left = ShinyType::MAYBE_SHINY;
        m_shininess_right = ShinyType::MAYBE_SHINY;
        return;
    }

    //  Shiny. Someone claims it.
    m_shininess_left = m_shininess.left_is_shiny ? ShinyType::UNKNOWN_SHINY : ShinyType::NOT_SHINY;
    m_shininess_right = m_shininess.right_is_shiny ? ShinyType::UNKNOWN_SHINY : ShinyType::NOT_SHINY;
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
    case ShinyType::NOT_SHINY:
        return false;
    case ShinyType::MAYBE_SHINY:
    case ShinyType::UNKNOWN_SHINY:
    case ShinyType::STAR_SHINY:
    case ShinyType::SQUARE_SHINY:
        return true;
    }
    return false;
}

std::set<std::string> StandardEncounterDetection::read_name(const ImageViewRGB32& screen, const ImageFloatBox& box){
    ImageViewRGB32 image = extract_box_reference(screen, box);

    std::set<std::string> ret;

    OCR::StringMatchResult result = PokemonNameReader::instance().read_substring(
        m_stream.logger(), m_language, image,
        OCR::BLACK_TEXT_FILTERS()
    );
    if (result.results.empty()){
        dump_image(
            m_stream.logger(), ProgramInfo(),
            "StandardEncounterDetection-NameOCR-" + language_data(m_language).code,
            screen,
            &m_stream.history()
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
bool StandardEncounterDetection::run_overrides(
    EncounterActionFull& action,
    const std::vector<std::unique_ptr<EncounterFilterOverride>>& overrides,
    const PokemonDetection& pokemon, ShinyType side_shiny
) const{
    if (!pokemon.exists || !pokemon.detection_enabled){
        return false;
    }
    bool triggered = false;
    for (const std::unique_ptr<EncounterFilterOverride>& override : overrides){
        //  Not a token match.
        if (pokemon.slugs.find(override->pokemon.slug()) == pokemon.slugs.end()){
            continue;
        }

        ShinyType shiny = side_shiny;
        if (shiny == ShinyType::MAYBE_SHINY){
//            actions.emplace_back(EncounterAction::StopProgram, "");
            throw_and_log<FatalProgramException>(
                m_stream.logger(), ErrorReport::SEND_ERROR_REPORT,
                "Cannot run encounter actions due to low confidence shiny detection.",
                m_stream
            );
        }

        //  Matched the filter.
        if (filter_match(shiny, override->shininess)){
            triggered = true;
            action = {override->action, override->pokeball.slug(), override->ball_limit};
        }
    }
    return triggered;
}


EncounterActionFull StandardEncounterDetection::get_action_singles(){
    if (m_shininess_right == ShinyType::UNKNOWN){
        return {EncounterAction::RunAway, "", 999};
    }

    ShinyFilter shiny_filter = m_filter.SHINY_FILTER;

    EncounterActionFull default_action;
    default_action.action = filter_match(m_shininess_right, shiny_filter)
        ? EncounterAction::StopProgram
        : EncounterAction::RunAway;

    const std::vector<std::unique_ptr<EncounterFilterOverride>>& overrides = m_filter.FILTER_TABLE.copy_snapshot();
    if (m_language != Language::None && !overrides.empty()){
        run_overrides(default_action, overrides, m_pokemon_right, m_shininess_right);
    }

    m_stream.log("Action: " + default_action.to_str());

    return default_action;
}
EncounterActionFull StandardEncounterDetection::get_action_doubles(){
    ShinyFilter shiny_filter = m_filter.SHINY_FILTER;

    EncounterActionFull action_left;
    action_left.action = filter_match(m_shininess_left, shiny_filter)
        ? EncounterAction::StopProgram
        : EncounterAction::RunAway;

    EncounterActionFull action_right;
    action_right.action = filter_match(m_shininess_right, shiny_filter)
        ? EncounterAction::StopProgram
        : EncounterAction::RunAway;

    const std::vector<std::unique_ptr<EncounterFilterOverride>>& overrides = m_filter.FILTER_TABLE.copy_snapshot();
    if (m_language != Language::None && !overrides.empty()){
        run_overrides(action_left, overrides, m_pokemon_left, m_shininess_left);
        run_overrides(action_right, overrides, m_pokemon_right, m_shininess_right);
    }

    std::string str_left = "Left " + STRING_POKEMON + ": " + action_left.to_str();
    std::string str_right = "Right " + STRING_POKEMON + ": " + action_right.to_str();
    m_stream.log(str_left);
    m_stream.log(str_right);

    //  If either action is stop program, we stop program.
    if (action_left.action == EncounterAction::StopProgram){
        return action_left;
    }
    if (action_right.action == EncounterAction::StopProgram){
        return action_right;
    }

    if (action_left != action_right){
        throw_and_log<FatalProgramException>(
            m_stream.logger(), ErrorReport::NO_ERROR_REPORT,
            "Conflicting actions requested.\n" + str_left + "\n" + str_right,
            m_stream
        );
    }

    bool auto_catch = false;
    auto_catch |= action_left.action == EncounterAction::ThrowBalls;
    auto_catch |= action_left.action == EncounterAction::ThrowBallsAndSave;
    auto_catch |= action_right.action == EncounterAction::ThrowBalls;
    auto_catch |= action_right.action == EncounterAction::ThrowBallsAndSave;

    //  Double battle and someone is set to auto-catch.
    if (auto_catch && m_double_battle){
        throw_and_log<FatalProgramException>(
            m_stream.logger(), ErrorReport::NO_ERROR_REPORT,
            "Cannot auto-catch in a double battle.",
            m_stream
        );
    }

    //  Otherwise, return the matching action.
    return action_right;
}

EncounterActionFull StandardEncounterDetection::get_action(){
    return m_double_battle ? get_action_doubles() : get_action_singles();
}







}
}
}
