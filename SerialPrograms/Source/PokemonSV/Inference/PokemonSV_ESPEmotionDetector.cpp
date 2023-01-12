/*  ESP Emotion Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonSV_ESPEmotionDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

ESPEmotionReader::ESPEmotionReader()
    : m_symbol_box(0.297, 0.137, 0.010, 0.016)
{}

void ESPEmotionReader::make_overlays(VideoOverlaySet& items) const {
    items.add(COLOR_RED, m_symbol_box);
}
Detection ESPEmotionReader::detect(const ImageViewRGB32& screen) const {
    ImageViewRGB32 symbol = extract_box_reference(screen, m_symbol_box);

    //Color ratio: R/(R+G+B), G/(R+G+B), B/(R+G+B)
    if (is_solid(symbol, { 0.567, 0.2, 0.232 }, 0.2, 40)) {
        return Detection::RED;
    }
    if (is_solid(symbol, { 0.529, 0.447, 0.0258 }, 0.2, 40)) {
        return Detection::YELLOW;
    }
    if (is_solid(symbol, { 0.132, 0.332, 0.536 }, 0.2, 40)) {
        return Detection::BLUE; //Sometimes picks up the grey as well but that works
    }
    if (is_solid(symbol, { 0.323, 0.491, 0.184 }, 0.2, 40)) {
        return Detection::GREEN;
    }
    if (is_solid(symbol, { 0.219, 0.355, 0.426 }, 0.2, 40)) {
        return Detection::GREY;
    }
    return Detection::NO_DETECTION;
}

ESPEmotionDetector::ESPEmotionDetector()
    : VisualInferenceCallback("ESPEmotionDetector")
    , m_last(Detection::NO_DETECTION)
{}

void ESPEmotionDetector::make_overlays(VideoOverlaySet& items) const {
    m_reader.make_overlays(items);
}
bool ESPEmotionDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp) {
    Detection result = m_reader.detect(frame);
    m_last.store(result, std::memory_order_release);
    return result != Detection::NO_DETECTION;
}

ESPStartDetector::ESPStartDetector()
    : VisualInferenceCallback("ESPStartDetector")
    , m_left_box(0.340, 0.140, 0.007, 0.068)
    , m_right_box(0.712, 0.141, 0.011, 0.066)
{}
void ESPStartDetector::make_overlays(VideoOverlaySet& items) const {
    items.add(COLOR_RED, m_left_box);
    items.add(COLOR_RED, m_right_box);
}
bool ESPStartDetector::detect(const ImageViewRGB32& frame) {
    ImageViewRGB32 left_image = extract_box_reference(frame, m_left_box);
    ImageViewRGB32 right_image = extract_box_reference(frame, m_right_box);
    if (is_solid(left_image, { 0.332, 0.335, 0.332 }) && is_solid(right_image, { 0.332, 0.335, 0.332 })) {
        return true;
    }
    return false;
}
bool ESPStartDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp) {
    return detect(frame);
}

ESPShowNewEmotionDetector::ESPShowNewEmotionDetector()
    : VisualInferenceCallback("ESPShowNewEmotionDetector")
    , m_left_box(0.340, 0.140, 0.007, 0.068)
    , m_right_box(0.712, 0.141, 0.011, 0.066)
{}
void ESPShowNewEmotionDetector::make_overlays(VideoOverlaySet& items) const {
    items.add(COLOR_RED, m_left_box);
    items.add(COLOR_RED, m_right_box);
}
bool ESPShowNewEmotionDetector::detect(const ImageViewRGB32& frame) {
    ImageViewRGB32 left_image = extract_box_reference(frame, m_left_box);
    ImageViewRGB32 right_image = extract_box_reference(frame, m_right_box);
    if (is_solid(left_image, { 0.332, 0.335, 0.332 }) && is_solid(right_image, { 0.332, 0.335, 0.332 })) {
        return false;
    }
    return true;
}
bool ESPShowNewEmotionDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp) {
    return detect(frame);
}

}
}
}
