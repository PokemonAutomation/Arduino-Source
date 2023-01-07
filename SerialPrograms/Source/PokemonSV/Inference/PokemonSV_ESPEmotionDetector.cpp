/*  ESP Emotion Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/ImageTools/ColorClustering.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Inference/InferenceThrottler.h"
#include "PokemonSV_ESPEmotionDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

ESPEmotionDetector::ESPEmotionDetector(Logger& logger, VideoOverlay& overlay)
    : m_logger(logger)
    , m_symbol_box(overlay, { 0.297, 0.137, 0.010, 0.016 }) //emotion
{}

ESPEmotionDetector::Detection ESPEmotionDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 symbol = extract_box_reference(screen, m_symbol_box);

    //Color ratio: R/(R+G+B), G/(R+G+B), B/(R+G+B)
    if (is_solid(symbol, { 0.567, 0.2, 0.232 }, 0.1, 10)) {
        return Detection::RED;
    }
    if (is_solid(symbol, { 0.529, 0.447, 0.0258}, 0.1, 10)) {
        return Detection::YELLOW;
    }
    if (is_solid(symbol, { 0.132, 0.332, 0.536 }, 0.1, 10)) {
        return Detection::BLUE; //Picks up the grey as well but that works
    }
    if (is_solid(symbol, { 0.323, 0.491, 0.184 }, 0.1, 10)) {
        return Detection::GREEN;
    }
    if (is_solid(symbol, { 0.219, 0.355, 0.426 }, 0.1, 10)) {
        return Detection::GREY;
    }
    return Detection::NO_DETECTION;
}
ESPEmotionDetector::Detection ESPEmotionDetector::wait_for_detection(
    CancellableScope& scope, VideoFeed& feed,
    std::chrono::seconds timeout
){
    Detection last_detection = Detection::NO_DETECTION;
    size_t confirmations = 0;

    InferenceThrottler throttler(timeout);
    while (true){
        scope.throw_if_cancelled();

        Detection detection = detect(feed.snapshot());
        if (detection == last_detection){
            confirmations++;
        }else{
            last_detection = detection;
            confirmations = 0;
        }
        //Don't need multiple confirmations as this isn't a shiny detector
        if (last_detection != Detection::NO_DETECTION && confirmations >= 1){
            break;
        }
        if (throttler.end_iteration(scope)){
            last_detection = Detection::NO_DETECTION;
            break;
        }
    }

    switch (last_detection){
    case Detection::NO_DETECTION:
        m_logger.log("ESPEmotionDetector: End game - Nothing found after timeout.", COLOR_BLACK);
        break;
    case Detection::RED:
        m_logger.log("ESPEmotionDetector: Angry - Red - Press X", COLOR_BLACK);
        break;
    case Detection::YELLOW:
        m_logger.log("ESPEmotionDetector: Joy - Yellow - Press A", COLOR_BLACK);
        break;
    case Detection::BLUE:
        m_logger.log("ESPEmotionDetector: Surprised - Blue - Press B", COLOR_BLACK);
        break;
    case Detection::GREEN:
        m_logger.log("ESPEmotionDetector: Excited - Green - Press Y", COLOR_BLACK);
        break;
    case Detection::GREY:
        m_logger.log("ESPEmotionDetector: Grey - Mash though dialog", COLOR_BLACK);
        break;
    }
    return last_detection;
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
