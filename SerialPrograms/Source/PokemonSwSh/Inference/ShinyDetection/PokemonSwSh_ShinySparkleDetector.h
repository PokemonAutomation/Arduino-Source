/*  Shiny Sparkle Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinySparkleDetector_H
#define PokemonAutomation_PokemonSwSh_ShinySparkleDetector_H

#include <deque>
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"
#include "Pokemon/Pokemon_Types.h"
#include "PokemonSwSh_ShinyTrigger.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;


class ShinySparkleDetector : public VisualInferenceCallbackWithCommandStop{
public:
    ShinySparkleDetector(
        Logger& logger, VideoOverlay& overlay,
        const ImageFloatBox& detection_box,
        double detection_threshold = 1.0
    );

    ShinyType results() const;

    virtual bool on_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    Logger& m_logger;
    VideoOverlay& m_overlay;
    InferenceBoxScope m_detection_box;
    double m_detection_threshold;

    ShinyImageAlpha m_image_alpha;
    std::deque<InferenceBoxScope> m_detection_overlays;
};


}
}
}
#endif
