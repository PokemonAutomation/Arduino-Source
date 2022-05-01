/*  Status Info Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "PokemonLA_StatusInfoScreenDetector.h"
#include "PokemonLA_CommonColorCheck.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


StatusInfoScreenDetector::StatusInfoScreenDetector()
    : VisualInferenceCallback("StatusInfoScreenDetector")
    , m_detected(0)
    , m_shiny_box(0.726, 0.133, 0.015, 0.023)
    , m_alpha_box(0.756, 0.137, 0.005, 0.005)
    , m_gender_box(0.777, 0.138, 0.001, 0.015)
    , m_name_box(0.525, 0.130, 0.100, 0.038)
{}

void StatusInfoScreenDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_BLUE, m_shiny_box);
    items.add(COLOR_RED, m_alpha_box);
    items.add(COLOR_PURPLE, m_gender_box);
    items.add(COLOR_BLACK, m_name_box);
}


bool StatusInfoScreenDetector::process_frame(
    const QImage& frame,
        std::chrono::system_clock::time_point timestamp
){
    const ImageStats shiny_box = image_stats(extract_box_reference(frame, m_shiny_box));
    if(!is_solid(shiny_box, {0.333333, 0.333333, 0.333333}, 0.2, 20)){
       m_detected = m_detected + 1;
       m_pokemon.is_shiny = true;
    }

    const ImageStats alpha_box = image_stats(extract_box_reference(frame, m_alpha_box));
    if(is_solid(alpha_box, {0.501968, 0.157480, 0.137795}, 0.2, 20)){
       m_detected = m_detected + 2;
       m_pokemon.is_alpha = true;
    }

    const ImageStats gender_box = image_stats(extract_box_reference(frame, m_gender_box));
    if(is_solid(gender_box, {0.333333, 0.333333, 0.333333}, 0.1, 10)){
        m_pokemon.gender = "genderless";
    }
    else if(is_solid(gender_box, { 0.233,0.265,0.502}, 0.3, 30)){
       m_pokemon.gender = "male";
    }
    else if(is_solid(gender_box, { 0.617,0.102,0.281}, 0.3, 30)){
       m_pokemon.gender = "female";
    }
    else{
        m_pokemon.gender = "UKNOWN";
    }

    return false;
}

void StatusInfoScreenDetector::get_pokemon_name(ConsoleHandle& console, QImage frame, Language language){

    QImage image = extract_box_copy(frame, m_name_box);

    OCR::StringMatchResult result = Pokemon::PokemonNameReader::instance().read_substring(
        console, language, image,
        OCR::BLACK_TEXT_FILTERS()
    );
    m_pokemon.name = "UNIDENTIFIED";

    for (const auto& item : result.results){
        m_pokemon.name = item.second.original_text.trimmed();
    }
}


}
}
}
