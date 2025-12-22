/*  Hyperspace Calorie Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "PokemonLZA_HyperspaceCalorieDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


HyperspaceCalorieDetector::HyperspaceCalorieDetector(Logger& logger)
: m_logger(logger)
, m_calorie_number_box{0.036, 0.118, 0.119, 0.059}
{}

void HyperspaceCalorieDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_WHITE, m_calorie_number_box);
}

bool HyperspaceCalorieDetector::detect(const ImageViewRGB32& screen){
    const ImageViewRGB32 calorie_image_crop = extract_box_reference(screen, m_calorie_number_box);

    const bool text_inside_range = true;
    const bool prioritize_numeric_only_results = true;
    const size_t width_max = SIZE_MAX;
    // The calorie crop includes the "," in Calorie numbers like "1,000".
    // We have to use `min_digit_area` to filter out "," when doing OCR.
    // The min digit area computation is that any dot with size smaller than calorie_image_crop.height()/5 is filtered out when OCR.
    const size_t min_digit_area = calorie_image_crop.height()*calorie_image_crop.height() / 25;
    m_calorie_number = 0;
    int number = OCR::read_number_waterfill_multifilter(m_logger, calorie_image_crop,
        {
            {0xff808080, 0xffffffff},
            {0xffa0a0a0, 0xffffffff},
            {0xffc0c0c0, 0xffffffff},
            {0xffe0e0e0, 0xffffffff},
            {0xfff0f0f0, 0xffffffff},
        },
        text_inside_range, prioritize_numeric_only_results, width_max, min_digit_area
    );
    if (number <= 0 || number > 9999){
        return false;
    }
    m_calorie_number = static_cast<uint16_t>(number);
    return true;
}




}
}
}
