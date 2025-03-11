/*  Status Info Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLA_CommonColorCheck.h"
#include "PokemonLA_StatusInfoScreenDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



PokemonDetails read_status_info(
    Logger& logger, VideoOverlay& overlay,
    const ImageViewRGB32& frame,
    Language language
){
    OverlayBoxScope shiny_box(overlay, {0.726, 0.133, 0.015, 0.023}, COLOR_BLUE);
    OverlayBoxScope alpha_box(overlay, {0.750, 0.133, 0.015, 0.023}, COLOR_RED);
    OverlayBoxScope gender_box(overlay, {0.777, 0.138, 0.001, 0.015}, COLOR_PURPLE);
    OverlayBoxScope name_box(overlay, {0.525, 0.130, 0.100, 0.038}, COLOR_BLACK);

    PokemonDetails ret;

    {
        const ImageStats shiny_box_stats = image_stats(extract_box_reference(frame, shiny_box));
        // std::cout << "ImageStats " << shiny_box_stats.average << "  " << shiny_box_stats.stddev << std::endl;
        const auto& stddev = shiny_box_stats.stddev;
        const double max_stddev = std::max(std::max(stddev.r, stddev.g), stddev.b);
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

    ImageViewRGB32 image = extract_box_reference(frame, name_box);

    OCR::StringMatchResult result = Pokemon::PokemonNameReader::instance().read_substring(
        logger, language, image,
        OCR::BLACK_TEXT_FILTERS()
    );

    for (auto& item : result.results){
        ret.name_candidates.insert(std::move(item.second.token));
    }

    return ret;
}







}
}
}
