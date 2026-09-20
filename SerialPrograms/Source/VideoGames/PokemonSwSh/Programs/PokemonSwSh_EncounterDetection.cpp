/*  Encounter Detection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "Common/NintendoSwitch/NintendoSwitch_Protocol_PushButtons.h"
//#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonTools/Async/InterruptableCommands.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh_EncounterDetection.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


StandardEncounterDetection::StandardEncounterDetection(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    Language language,
    const EncounterFilterOption2& filter,
    ShinyType shininess,
    std::chrono::milliseconds read_name_delay
)
    : m_env(env)
    , m_stream(stream)
    , m_context(context)
    , m_language(language)
    , m_filter(filter)
    , m_shininess(shininess)
    , m_read_name_delay(read_name_delay)
{}
bool StandardEncounterDetection::is_shiny() const{
    switch (m_shininess){
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
const std::set<std::string>* StandardEncounterDetection::candidates(){
    if (m_language == Language::None){
        m_name_read = true;
        return nullptr;
    }

    if (m_name_read){
        return &m_candidates;
    }

    OverlayBoxScope box(m_stream.overlay(), ImageFloatBox(0.76, 0.04, 0.15, 0.044));
    m_context.wait_for(m_read_name_delay);

    VideoSnapshot screen = m_stream.video().snapshot();
    ImageViewRGB32 frame = extract_box_reference(screen, box);

    OCR::StringMatchResult result = PokemonNameReader::instance().read_substring(
        m_stream.logger(), m_language, frame,
        OCR::BLACK_TEXT_FILTERS()
    );
    if (result.results.empty()){
        dump_image(
            m_stream.logger(), m_env.program_info(),
            "StandardEncounterDetection-NameOCR-" + language_data(m_language).code,
            screen
        );
    }else{
        m_candidates.clear();
        for (const auto& item : result.results){
            m_candidates.insert(item.second.token);
        }
    }

    m_name_read = true;
    return &m_candidates;
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
    case ShinyFilter::ANY_SHINY:
        return detection != ShinyType::NOT_SHINY;
    case ShinyFilter::STAR_ONLY:
        return detection == ShinyType::STAR_SHINY || detection == ShinyType::UNKNOWN_SHINY;
    case ShinyFilter::SQUARE_ONLY:
        return detection == ShinyType::SQUARE_SHINY || detection == ShinyType::UNKNOWN_SHINY;
    case ShinyFilter::NOTHING:
        return false;
    }

    return false;
}

EncounterActionFull StandardEncounterDetection::get_action(){
    if (m_shininess == ShinyType::UNKNOWN){
        return {EncounterAction::RunAway, ""};
    }

    EncounterActionFull action;
    action.action = filter_match(m_shininess, m_filter.SHINY_FILTER)
        ? EncounterAction::StopProgram
        : EncounterAction::RunAway;

//    const std::vector<EncounterFilterOverride>& overrides = m_filter.overrides();
    std::vector<std::unique_ptr<EncounterFilterOverride>> overrides = m_filter.FILTER_TABLE.copy_snapshot();

    if (overrides.empty()){
        return action;
    }

    const std::set<std::string>* candidates = this->candidates();
    if (candidates != nullptr){
        for (const std::unique_ptr<EncounterFilterOverride>& override : overrides){
            //  Not a token match.
            if (candidates->find(override->pokemon.slug()) == candidates->end()){
                continue;
            }

//            cout << "m_shininess = " << (int)m_shininess << " : " << (int)(ShinyFilter)override->shininess << endl;

            //  Not a shiny filter match.
            if (!filter_match(m_shininess, override->shininess)){
                continue;
            }

            action.action = override->action;
            action.pokeball_slug = override->pokeball.slug();
            action.ball_limit = override->ball_limit;
        }
    }

    return action;
}




}
}
}

