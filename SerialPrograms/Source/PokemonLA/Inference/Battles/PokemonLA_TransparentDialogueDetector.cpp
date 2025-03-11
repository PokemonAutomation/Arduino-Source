/*  Transparent Dialogue Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonLA_TransparentDialogueDetector.h"

#include <chrono>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


TransparentDialogueDetector::TransparentDialogueDetector(
    Logger& logger, VideoOverlay& overlay,
    bool stop_on_detected
)
    : VisualInferenceCallback("TransparentDialogueDetector")
    , m_arrow_detector(logger, overlay, stop_on_detected)
    , m_ellipse_detector(logger, overlay, std::chrono::milliseconds(0), stop_on_detected)
{}


void TransparentDialogueDetector::make_overlays(VideoOverlaySet& items) const{
    m_arrow_detector.make_overlays(items);
    m_ellipse_detector.make_overlays(items);
}


bool TransparentDialogueDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    bool stop = m_arrow_detector.process_frame(frame, timestamp);
    bool detected = m_arrow_detector.detected();
    if (detected){
        stop = stop && m_ellipse_detector.process_frame(frame, timestamp);
        detected = m_ellipse_detector.detected();
    }

    m_detected.store(detected, std::memory_order_release);

    return stop;
}





}
}
}
