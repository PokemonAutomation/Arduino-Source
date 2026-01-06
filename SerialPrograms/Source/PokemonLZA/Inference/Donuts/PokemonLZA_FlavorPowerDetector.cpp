/*  Donut Powers Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/ImageCropper.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "CommonTools/OCR/OCR_SmallDictionaryMatcher.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonLZA_FlavorPowerDetector.h"
// #include "CommonTools/ImageMatch/ImageMatchResult.h"
// #include "CommonTools/ImageMatch/CroppedImageDictionaryMatcher.h"

#include <format>
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


// OCR flavor power texts
class FlavorPowerReader : public OCR::SmallDictionaryMatcher{
public:
    static constexpr double MAX_LOG10P = -1.40;
    static constexpr double MAX_LOG10P_SPREAD = 0.50;

public:
    static FlavorPowerReader& instance();

    OCR::StringMatchResult read_substring(
        Logger& logger,
        Language language,
        const ImageViewRGB32& image,
        double min_text_ratio = 0.01, double max_text_ratio = 0.50
    ) const;

private:
    FlavorPowerReader();
};


FlavorPowerReader& FlavorPowerReader::instance(){
    static FlavorPowerReader reader;
    return reader;
}

FlavorPowerReader::FlavorPowerReader()
    : SmallDictionaryMatcher("PokemonLZA/Donuts/flavor_powers.json")
{}

OCR::StringMatchResult FlavorPowerReader::read_substring(
    Logger& logger,
    Language language,
    const ImageViewRGB32& image,
    double min_text_ratio, double max_text_ratio
) const{
    return match_substring_from_image_multifiltered(
        &logger, language, image, OCR::BLACK_TEXT_FILTERS(),
        MAX_LOG10P, MAX_LOG10P_SPREAD,
        min_text_ratio, max_text_ratio,
        OCR::PageSegMode::SINGLE_LINE
    );
}


FlavorPowerIconDetector::FlavorPowerIconDetector(Logger& logger, int position)
    : m_logger(logger)
    , m_icon_box(0.081, 0.764 + 0.045*position, 0.043, 0.034)
    , m_empty_space_after_number(0.1125, 0.7745 + 0.045*position, 0.005, 0.013)
    , m_subtype_free_number_box(0.102, 0.77 + 0.046*position, 0.011, 0.02)
    , m_subtype_number_box(0.108, 0.77 + 0.046*position, 0.011, 0.02)
{}


void FlavorPowerIconDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_WHITE, m_empty_space_after_number);
}


int FlavorPowerIconDetector::detect(const ImageViewRGB32& screen){
    const ImageStats icon_box_stats = image_stats(extract_box_reference(screen, m_icon_box));
    if (is_solid(icon_box_stats, {0.342,0.339,0.319})){
        // this flavor power is empty
        return -1;
    }

    const ImageStats empty_space_stats = image_stats(extract_box_reference(screen, m_empty_space_after_number));

    auto ocr_number = [&](const ImageViewRGB32 image_crop) -> int{
        const bool text_inside_range = false;
        const bool prioritize_numeric_only_results = true;
        const size_t width_max = SIZE_MAX;
        const size_t min_digit_area = image_crop.height()*image_crop.height() / 10;
        int number = OCR::read_number_waterfill_multifilter(m_logger, image_crop,
            {
                {0xff000000, 0xff707070},
                {0xff000000, 0xff808080},
                {0xff000000, 0xff909090},
                {0xff000000, 0xffa0a0a0},
            },
            text_inside_range, prioritize_numeric_only_results, width_max, min_digit_area
        );
        if (number < 1 || number > 3){
            throw_and_log<OperationFailedException>(
                m_logger, ErrorReport::SEND_ERROR_REPORT,
                "DontPowerSpriteDetector::detect(): Unable to OCR power level. Got: " + std::to_string(number)
            );
        }
        return number;
    };

    if (is_black(empty_space_stats, 250, 50)){
        // the power has no sub-types
        const ImageViewRGB32 image_crop = extract_box_reference(screen, m_subtype_free_number_box);
        int number = ocr_number(image_crop);
        return number;
    } else{
        // we have sub-types
        const ImageViewRGB32 image_crop = extract_box_reference(screen, m_subtype_number_box);
        int number = ocr_number(image_crop);
        return number;
    }
}


FlavorPowerDetector::FlavorPowerDetector(Logger& logger, Color color, Language language, int position)
    : m_logger(logger), m_color(color), m_language(language)
    , m_position(position), m_ocr_box(0.131, 0.759 + 0.045*position, 0.260, 0.043)
    , m_icon_detector(logger, position)
{}

void FlavorPowerDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_ocr_box);
    m_icon_detector.make_overlays(items);
}

bool FlavorPowerDetector::detect(const ImageViewRGB32& screen){
    return !detect_power(screen).empty();
}

std::string FlavorPowerDetector::detect_power(const ImageViewRGB32& screen){
    const int detected_power_level = m_icon_detector.detect(screen);
    if (detected_power_level < 0){ // empty flavor power text
        return "";
    }
    std::multimap<double, std::string> results;
    /*
    ImageRGB32 quest_label = to_blackwhite_rgb32_range(
        extract_box_reference(screen, m_ocr_box),
        true,
        combine_rgb(198, 198, 198), combine_rgb(255, 255, 255)
    );*/
    ImageViewRGB32 quest_label = extract_box_reference(screen, m_ocr_box);
    // quest_label.save("quest_label.png");

    OCR::StringMatchResult ocr_result = FlavorPowerReader::instance().read_substring(
        m_logger, m_language, quest_label
    );
    ocr_result.clear_beyond_log10p(FlavorPowerReader::MAX_LOG10P);
    ocr_result.clear_beyond_spread(FlavorPowerReader::MAX_LOG10P_SPREAD);
    if (!ocr_result.results.empty()){
        for (const auto& result : ocr_result.results){
            if (result.second.token.size() <= 1){
                throw InternalProgramError(
                    &m_logger, PA_CURRENT_FUNCTION,
                    "FlavorPowerDetector::detect_power(): OCR match result has near empty output. Got " + result.second.original_text
                );
            }
            results.emplace(result.first, result.second.token);
        }
            
    }

    if (results.empty()){
        return "";
    }
    else if (results.size() == 1){
        return results.begin()->second;
    }

    auto get_all_results = [&]() -> std::string{
        std::string ret = "";
        for (const auto& result : results){
            ret += std::format("({}: {}), ", result.first, result.second);
        }
        return ret.substr(0, ret.size() - 2); // -2 to remove the last ", "
    };

    // More than one match result. Use Flavor Power Icon detector to disambiguate

    // Check if all results are the same except for the last character
    std::string base_string;
    bool all_same_except_last = true;

    for (const auto& result : results){
        const std::string& slug = result.second;
        // Get the base string (all except last character)
        std::string current_base = slug.substr(0, slug.length() - 1);

        if (base_string.empty()){
            base_string = current_base;
        } else if (base_string != current_base){
            all_same_except_last = false;
            break;
        }
    }

    if (!all_same_except_last){
        throw_and_log<OperationFailedException>(
            m_logger, ErrorReport::SEND_ERROR_REPORT,
            "FlavorPowerDetector::detect_power(): Unable to read selected item. "
            "Ambiguous or multiple results: " + get_all_results() + "\n" + 
            language_warning(m_language)
        );
    }

    // All results differ only in last character, use icon detector to disambiguate
    
    // Convert power level (1, 2, 3) to character ('1', '2', '3')
    char expected_last_char = (char)('0' + detected_power_level);
    // Find the result with matching last character
    for (const auto& result : results){
        const std::string& slug = result.second;
        if (!slug.empty() && slug.back() == expected_last_char){
            return slug;
        }
    }
    throw_and_log<OperationFailedException>(
        m_logger, ErrorReport::SEND_ERROR_REPORT,
        "FlavorPowerDetector::detect_power(): Detected power level icon is "
        + std::to_string(detected_power_level)
        + " while getting mismatched power OCR: " + get_all_results() + 
        ".\n" + language_warning(m_language)
    );
}


FlavorPowerWatcher::~FlavorPowerWatcher() = default;

FlavorPowerWatcher::FlavorPowerWatcher(Logger& logger, Color color, Language language, int position)
    : VisualInferenceCallback("FlavorPowerWatcher")
    , m_detector(logger, color, language, position)
{}

void FlavorPowerWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}

bool FlavorPowerWatcher::process_frame(const ImageViewRGB32& screen, WallClock timestamp){
    m_quest_name = m_detector.detect_power(screen);
    if (m_quest_name.empty()){
        return false;
    }

    return true;
}


}
}
}
