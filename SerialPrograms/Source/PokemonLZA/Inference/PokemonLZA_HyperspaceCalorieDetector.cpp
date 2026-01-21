/*  Hyperspace Calorie Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "CommonTools/OCR/OCR_RawOCR.h"
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

void HyperspaceCalorieDetector::warm_ocr(){
    // Ensure the number used here matches the actual filter size used in detect()
    OCR::ensure_instances(Language::English, 5);
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
    // Ensure the `filters.size()` equals to the value used to warm up OCR object cache in warm_ocr()
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {0xff808080, 0xffffffff},
        {0xffa0a0a0, 0xffffffff},
        {0xffc0c0c0, 0xffffffff},
        {0xffe0e0e0, 0xffffffff},
        {0xfff0f0f0, 0xffffffff},
    };
    int number = OCR::read_number_waterfill_multifilter(
        m_logger,
        GlobalThreadPools::realtime_inference(),
        calorie_image_crop, filters,
        text_inside_range, prioritize_numeric_only_results, width_max, min_digit_area
    );
    if (number <= 0 || number > 9999){
        return false;
    }
    m_calorie_number = static_cast<uint16_t>(number);
    return true;
}

HyperspaceCalorieWatcher::HyperspaceCalorieWatcher(Logger& logger)
: HyperspaceCalorieDetector(logger), VisualInferenceCallback("HyperspaceCalorieWatcher")
{}

void HyperspaceCalorieWatcher::make_overlays(VideoOverlaySet& items) const{
    HyperspaceCalorieDetector::make_overlays(items);
}

bool HyperspaceCalorieWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return detect(frame);
}

HyperspaceCalorieLimitWatcher::HyperspaceCalorieLimitWatcher(Logger& logger, uint16_t calorie_limit)
: HyperspaceCalorieDetector(logger), VisualInferenceCallback("HyperspaceCalorieLimitWatcher")
, m_calorie_limit(calorie_limit)
{}

void HyperspaceCalorieLimitWatcher::make_overlays(VideoOverlaySet& items) const{
    HyperspaceCalorieDetector::make_overlays(items);
}

bool HyperspaceCalorieLimitWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    bool detected = detect(frame);
    if (!detected){
        m_start_of_detection = WallClock::min();
        return false;
    }
    const uint16_t calorie = calorie_number();
    if (calorie > m_calorie_limit){
        m_start_of_detection = WallClock::min();
        return false;
    }

    // Debug possible Calorie number reading failure:
    if (m_last_calorie_images.size() == 0){
        m_last_calorie_images.emplace_front(calorie, frame.copy());
    }else{
        uint16_t last_calorie = m_last_calorie_images.front().first;
        if (calorie > last_calorie){
            // calorie shouldn't go up!
            for(const auto& p : m_last_calorie_images){
                p.second.save(DEBUG_PATH() + "/HyperspaceCalorieLimitWatcher/PossibleWrongCalorie_" + std::to_string(p.first) + ".png");
            }
        }
        else if (calorie < last_calorie){
            m_last_calorie_images.emplace_front(calorie, frame.copy());
            if (m_last_calorie_images.size() > 3){
                m_last_calorie_images.pop_back();
            }
        }
    }

    if (m_start_of_detection == WallClock::min()){
        m_start_of_detection = timestamp;
    }

    if (timestamp - m_start_of_detection >= Milliseconds(300) && calorie <= m_calorie_limit){
        return true;
    }

    return false;
}



}
}
}
