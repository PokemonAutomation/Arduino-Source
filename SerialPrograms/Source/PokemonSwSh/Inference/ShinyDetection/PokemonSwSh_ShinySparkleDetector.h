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
#include "Pokemon/Pokemon_DataTypes.h"
#include "PokemonSwSh_ShinyTrigger.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;


class ShinySparkleDetector : public VisualInferenceCallback{
public:
    ShinySparkleDetector(
        Logger& logger, VideoOverlay& overlay,
        const ImageFloatBox& detection_box,
        double detection_threshold = 1.0
    );

    ShinyType results() const;

    virtual void make_overlays(OverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    Logger& m_logger;
    VideoOverlay& m_overlay;
    ImageFloatBox m_detection_box;
    double m_detection_threshold;

    ShinyImageAlpha m_image_alpha;
    std::deque<InferenceBoxScope> m_detection_overlays;
};


}
}
}
#endif
