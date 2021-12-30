/*  Shiny Sparkle Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSwSh_ShinySparkleDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



ShinySparkleDetector::ShinySparkleDetector(
    Logger& logger, VideoOverlay& overlay,
    const ImageFloatBox& detection_box,
    double detection_threshold
)
    : m_logger(logger)
    , m_overlay(overlay)
    , m_detection_box(detection_box)
    , m_detection_threshold(detection_threshold)
{}
void ShinySparkleDetector::make_overlays(OverlaySet& items) const{
    items.add(COLOR_RED, m_detection_box);
}


ShinyType ShinySparkleDetector::results() const{
    double alpha = m_image_alpha.shiny;

    m_logger.log(
        "ShinySparkleDetector: Overall Alpha = " + QString::number(alpha) +
        ", Star Alpha = " + QString::number(m_image_alpha.star) +
        ", Square Alpha = " + QString::number(m_image_alpha.square),
        COLOR_PURPLE
    );

    if (alpha < m_detection_threshold){
        m_logger.log("ShinySparkleDetector: Not Shiny.", COLOR_PURPLE);
        return ShinyType::NOT_SHINY;
    }
    if (m_image_alpha.star > 0 && m_image_alpha.star > m_image_alpha.square * 2){
        m_logger.log("ShinySparkleDetector: Detected Star Shiny!", COLOR_BLUE);
        return ShinyType::STAR_SHINY;
    }
    if (m_image_alpha.square > 0 && m_image_alpha.square > m_image_alpha.star * 2){
        m_logger.log("ShinySparkleDetector: Detected Square Shiny!", COLOR_BLUE);
        return ShinyType::SQUARE_SHINY;
    }

    m_logger.log("ShinySparkleDetector: Detected Shiny! But ambiguous shiny type.", COLOR_BLUE);
    return ShinyType::UNKNOWN_SHINY;
}


bool ShinySparkleDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    QImage shiny_box = extract_box(frame, m_detection_box);

    ShinyImageDetection signatures;
    signatures.accumulate(shiny_box, timestamp.time_since_epoch().count(), &m_logger);

    ShinyImageAlpha frame_alpha = signatures.alpha();

    m_image_alpha.max(frame_alpha);

    if (frame_alpha.shiny > 0){
        if (frame_alpha.shiny >= m_detection_threshold){
            m_logger.log(
                "ShinyDetector: alpha = " + QString::number(frame_alpha.shiny) + " / "  + QString::number(m_image_alpha.shiny) + " (threshold exceeded)",
                COLOR_BLUE
            );
        }else{
            m_logger.log(
                "ShinyDetector: alpha = " + QString::number(frame_alpha.shiny) + " / "  + QString::number(m_image_alpha.shiny),
                COLOR_BLUE
            );
        }
    }

    m_detection_overlays.clear();
    for (const auto& item : signatures.balls){
        ImageFloatBox box = translate_to_parent(frame, m_detection_box, item);
        m_detection_overlays.emplace_back(m_overlay, box, COLOR_GREEN);
    }
    for (const auto& item : signatures.stars){
        ImageFloatBox box = translate_to_parent(frame, m_detection_box, item);
        m_detection_overlays.emplace_back(m_overlay, box, COLOR_GREEN);
    }
    for (const auto& item : signatures.squares){
        ImageFloatBox box = translate_to_parent(frame, m_detection_box, item);
        m_detection_overlays.emplace_back(m_overlay, box, COLOR_GREEN);
    }
    for (const auto& item : signatures.lines){
        ImageFloatBox box = translate_to_parent(frame, m_detection_box, item);
        m_detection_overlays.emplace_back(m_overlay, box, COLOR_GREEN);
    }

    return false;
}



}
}
}

