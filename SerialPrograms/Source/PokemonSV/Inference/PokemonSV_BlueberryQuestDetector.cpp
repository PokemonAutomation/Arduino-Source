/*  Blueberry Quest Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "PokemonSV/Inference/PokemonSV_BlueberryQuestReader.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "PokemonSV_BlueberryQuestDetector.h"

#include <iostream>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


//BlueberryQuestDetector::~BlueberryQuestDetector() = default;

BlueberryQuestDetector::BlueberryQuestDetector(Logger& logger, Color color, Language language, QuestPosition position)
    : m_logger(logger), m_color(color), m_language(language), m_position(position)
{
    switch(position){
    case QuestPosition::FIRST:
        m_box = ImageFloatBox(0.604, 0.218, 0.282, 0.095);
        break;
    case QuestPosition::SECOND:
        m_box = ImageFloatBox(0.604, 0.395, 0.282, 0.095);
        break;
    case QuestPosition::THIRD:
        m_box = ImageFloatBox(0.604, 0.572, 0.282, 0.095);
        break;
    case QuestPosition::FOURTH:
        m_box = ImageFloatBox(0.604, 0.749, 0.282, 0.095);
        break;
    }
}

void BlueberryQuestDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool BlueberryQuestDetector::detect(const ImageViewRGB32& screen) const{
    return !detect_quest(screen).empty();
}

std::string BlueberryQuestDetector::detect_quest(const ImageViewRGB32& screen) const{
    std::multimap<double, OCR::StringMatchData> results;

    ImageRGB32 quest_label = to_blackwhite_rgb32_range(
        extract_box_reference(screen, m_box),
        combine_rgb(198, 198, 198), combine_rgb(255, 255, 255), true
    );

    //quest_label.save("quest_label.png");

    OCR::StringMatchResult ocr_result = PokemonSV::BlueberryQuestReader::instance().read_substring(
        m_logger, m_language, quest_label,
        OCR::BLACK_TEXT_FILTERS()
    );
    ocr_result.clear_beyond_log10p(BlueberryQuestReader::MAX_LOG10P);
    ocr_result.clear_beyond_spread(BlueberryQuestReader::MAX_LOG10P_SPREAD);
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
            "BlueberryQuestDetector::detect_quest(): Unable to read selected item. Ambiguous or multiple results."
        );
    }

    return results.begin()->second.token;
}



BlueberryQuestWatcher::~BlueberryQuestWatcher() = default;

BlueberryQuestWatcher::BlueberryQuestWatcher(Logger& logger, Color color, VideoOverlay& overlay, Language language, BlueberryQuestDetector::QuestPosition position)
    : VisualInferenceCallback("BlueberryQuestWatcher")
    , m_overlay(overlay)
    , m_detector(logger, color, language, position)
{}

void BlueberryQuestWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}

bool BlueberryQuestWatcher::process_frame(const ImageViewRGB32& screen, WallClock timestamp){
    m_quest_name = m_detector.detect_quest(screen);
    if (m_quest_name.empty()){
        return false;
    }

    return true;
}








}
}
}
