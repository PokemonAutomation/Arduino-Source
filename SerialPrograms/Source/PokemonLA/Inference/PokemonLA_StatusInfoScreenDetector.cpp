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

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



PokemonDetails read_status_info(
    LoggerQt& logger, VideoOverlay& overlay,
    const QImage& frame,
    Language language
){
    InferenceBoxScope shiny_box(overlay, 0.726, 0.133, 0.015, 0.023, COLOR_BLUE);
    InferenceBoxScope alpha_box(overlay, 0.750, 0.133, 0.015, 0.023, COLOR_RED);
    InferenceBoxScope gender_box(overlay, 0.777, 0.138, 0.001, 0.015, COLOR_PURPLE);
    InferenceBoxScope name_box(overlay, 0.525, 0.130, 0.100, 0.038, COLOR_BLACK);

    PokemonDetails ret;

    {
        const ImageStats shiny_box_stats = image_stats(extract_box_reference(frame, shiny_box));
        // std::cout << "ImageStats " << shiny_box_stats.average << "  " << shiny_box_stats.stddev << std::endl;
        const auto& stddev = shiny_box_stats.stddev;
        const float max_stddev = std::max(std::max(stddev.r, stddev.g), stddev.b);
        if(!is_solid(shiny_box_stats, {0.333333, 0.333333, 0.333333}, 0.2, 15) || max_stddev > 8.){
            ret.is_shiny = true;
            logger.log("Detected Shiny!", COLOR_BLUE);
        }
    }

    const ImageStats alpha_stats = image_stats(extract_box_reference(frame, alpha_box));
    if (alpha_stats.stddev.sum() > 80 &&
        alpha_stats.average.r > alpha_stats.average.g + 30 &&
        alpha_stats.average.r > alpha_stats.average.b + 30
    ){
        ret.is_alpha = true;
        logger.log("Detected Alpha!", COLOR_BLUE);
    }

    const ImageStats gender_stats = image_stats(extract_box_reference(frame, gender_box));
//    cout << gender_stats.average << gender_stats.stddev << endl;
    if (is_solid(gender_stats, {0.333333, 0.333333, 0.333333}, 0.1, 10)){
        ret.gender = Gender::Genderless;
        logger.log("Gender: Genderless");
    }else if (gender_stats.average.b > gender_stats.average.g + 30 && gender_stats.average.b > gender_stats.average.r + 30){
        ret.gender = Gender::Male;
        logger.log("Gender: Male");
    }else if (gender_stats.average.r > gender_stats.average.g + 30 && gender_stats.average.r > gender_stats.average.b + 30){
        ret.gender = Gender::Female;
        logger.log("Gender: Female");
    }else{
        logger.log("Gender: Unable to detect", COLOR_RED);
    }

    if (language == Language::None){
        return ret;
    }

    QImage image = extract_box_copy(frame, name_box);

    OCR::StringMatchResult result = Pokemon::PokemonNameReader::instance().read_substring(
        logger, language, image,
        OCR::BLACK_TEXT_FILTERS()
    );

    for (auto& item : result.results){
        ret.name_candidates.insert(std::move(item.second.token));
    }

    return ret;
}



#if 0
StatusInfoScreenDetector::StatusInfoScreenDetector()
    : m_shiny_box(0.726, 0.133, 0.015, 0.023)
    , m_alpha_box(0.750, 0.133, 0.015, 0.023)
    , m_gender_box(0.777, 0.138, 0.001, 0.015)
    , m_name_box(0.525, 0.130, 0.100, 0.038)
{}

void StatusInfoScreenDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_BLUE, m_shiny_box);
    items.add(COLOR_RED, m_alpha_box);
    items.add(COLOR_PURPLE, m_gender_box);
    items.add(COLOR_BLACK, m_name_box);
}


void StatusInfoScreenDetector::detect(const QImage& frame){
    const ImageStats shiny_box = image_stats(extract_box_reference(frame, m_shiny_box));
    if(!is_solid(shiny_box, {0.333333, 0.333333, 0.333333}, 0.2, 20)){
       m_pokemon.is_shiny = true;
    }

    const ImageStats alpha_box = image_stats(extract_box_reference(frame, m_alpha_box));
//    cout << alpha_box.average << alpha_box.stddev << endl;
#if 0
    if (is_solid(alpha_box, {0.501968, 0.157480, 0.137795}, 0.2, 20)){
       m_pokemon.is_alpha = true;
    }
#else
    if (alpha_box.stddev.sum() > 80 &&
        alpha_box.average.r > alpha_box.average.g + 30 &&
        alpha_box.average.r > alpha_box.average.b + 30
    ){
        m_pokemon.is_alpha = true;
    }
#endif

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
}

void StatusInfoScreenDetector::get_pokemon_name(ConsoleHandle& console, QImage frame, Language language){

    QImage image = extract_box_copy(frame, m_name_box);

    OCR::StringMatchResult result = Pokemon::PokemonNameReader::instance().read_substring(
        console, language, image,
        OCR::BLACK_TEXT_FILTERS()
    );
    m_pokemon.name = "UNIDENTIFIED";

    for (const auto& item : result.results){
        m_pokemon.name = QString::fromStdString(item.second.token);
    }
}
#endif




}
}
}
