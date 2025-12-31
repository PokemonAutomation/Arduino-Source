/*  Donut Powers Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonLZA/Inference/Donuts/PokemonLZA_DonutPowerDetector.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/ImageCropper.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "CommonTools/OCR/OCR_SmallDictionaryMatcher.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonLZA_DonutPowerDetector.h"
// #include "CommonTools/ImageMatch/ImageMatchResult.h"
// #include "CommonTools/ImageMatch/CroppedImageDictionaryMatcher.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


// OCR flavor power texts
class DonutPowerReader : public OCR::SmallDictionaryMatcher{
public:
    static constexpr double MAX_LOG10P = -1.40;
    static constexpr double MAX_LOG10P_SPREAD = 0.50;

public:
    static DonutPowerReader& instance();

    OCR::StringMatchResult read_substring(
        Logger& logger,
        Language language,
        const ImageViewRGB32& image,
        const std::vector<OCR::TextColorRange>& text_color_ranges,
        double min_text_ratio = 0.01, double max_text_ratio = 0.50
    ) const;

private:
    DonutPowerReader();
};


DonutPowerReader& DonutPowerReader::instance(){
    static DonutPowerReader reader;
    return reader;
}

DonutPowerReader::DonutPowerReader()
    : SmallDictionaryMatcher("PokemonLZA/Donuts/flavor_powers.json")
{}

OCR::StringMatchResult DonutPowerReader::read_substring(
    Logger& logger,
    Language language,
    const ImageViewRGB32& image,
    const std::vector<OCR::TextColorRange>& text_color_ranges,
    double min_text_ratio, double max_text_ratio
) const{
    return match_substring_from_image_multifiltered(
        &logger, language, image, text_color_ranges,
        MAX_LOG10P, MAX_LOG10P_SPREAD, min_text_ratio, max_text_ratio
    );
}


class DonutPowerSpriteDetector {
public:
    DonutPowerSpriteDetector(int position);

private:
    ImageFloatBox m_empty_space_after_number;
};

DonutPowerSpriteDetector::DonutPowerSpriteDetector(int position)
    : m_empty_space_after_number(0.113, 0.775 + 0.045*position, 0.005, 0.013){}



DonutPowerDetector::DonutPowerDetector(Logger& logger, Color color, Language language, int position)
    : m_logger(logger), m_color(color), m_language(language)
    , m_position(position), m_ocr_box(0.131, 0.759 + 0.045*position, 0.260, 0.043)
{}

void DonutPowerDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_ocr_box);
}

bool DonutPowerDetector::detect(const ImageViewRGB32& screen){
    return !detect_power(screen).empty();
}

std::string DonutPowerDetector::detect_power(const ImageViewRGB32& screen) const{
    std::multimap<double, OCR::StringMatchData> results;
    /*
    ImageRGB32 quest_label = to_blackwhite_rgb32_range(
        extract_box_reference(screen, m_ocr_box),
        true,
        combine_rgb(198, 198, 198), combine_rgb(255, 255, 255)
    );*/
    ImageViewRGB32 quest_label = extract_box_reference(screen, m_ocr_box);
    // quest_label.save("quest_label.png");

    OCR::StringMatchResult ocr_result = DonutPowerReader::instance().read_substring(
        m_logger, m_language, quest_label,
        OCR::BLACK_TEXT_FILTERS()
    );
    ocr_result.clear_beyond_log10p(DonutPowerReader::MAX_LOG10P);
    ocr_result.clear_beyond_spread(DonutPowerReader::MAX_LOG10P_SPREAD);
    if (!ocr_result.results.empty()){
        for (const auto& result : ocr_result.results){
            results.emplace(result.first, result.second);
        }
            
    }

    if (results.empty()){
        return "";
    }

    if (results.size() > 1){
        throw_and_log<OperationFailedException>(
            m_logger, ErrorReport::SEND_ERROR_REPORT,
            "DonutPowerDetector::detect_power(): Unable to read selected item. Ambiguous or multiple results.\n" + language_warning(m_language)
        );
    }

    return results.begin()->second.token;
}



DonutPowerWatcher::~DonutPowerWatcher() = default;

DonutPowerWatcher::DonutPowerWatcher(Logger& logger, Color color, Language language, int position)
    : VisualInferenceCallback("DonutPowerWatcher")
    , m_detector(logger, color, language, position)
{}

void DonutPowerWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}

bool DonutPowerWatcher::process_frame(const ImageViewRGB32& screen, WallClock timestamp){
    m_quest_name = m_detector.detect_power(screen);
    if (m_quest_name.empty()){
        return false;
    }

    return true;
}


}
}
}
