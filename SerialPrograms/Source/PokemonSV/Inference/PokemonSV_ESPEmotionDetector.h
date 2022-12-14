/*  ESP Emotion Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_ESPEmotionDetector_H
#define PokemonAutomation_PokemonSV_ESPEmotionDetector_H

#include <chrono>
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"

namespace PokemonAutomation{
    class CancellableScope;
    class VideoFeed;
namespace NintendoSwitch{
namespace PokemonSV{


class ESPEmotionDetector{
public:
    enum Detection{
        NO_DETECTION,
        RED,
        YELLOW,
		BLUE,
		GREEN,
		GREY,
    };

public:
    ESPEmotionDetector(Logger& logger, VideoOverlay& overlay);

    Detection detect(const ImageViewRGB32& screen);
    Detection wait_for_detection(
        CancellableScope& scope, VideoFeed& feed,
        std::chrono::seconds timeout = std::chrono::seconds(10)
    );
private:
    Logger& m_logger;
    OverlayBoxScope m_symbol_box;
};

//This checks for the Dendra's dialog box
class ESPStartDetector : public VisualInferenceCallback {
public:
    ESPStartDetector();

    bool detect(const ImageViewRGB32& frame);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    ImageFloatBox m_left_box;
    ImageFloatBox m_right_box;
};

// This checks that Dendra's box vanished to shout the next emotion
class ESPShowNewEmotionDetector : public VisualInferenceCallback {
public:
    ESPShowNewEmotionDetector();

    bool detect(const ImageViewRGB32& frame);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    ImageFloatBox m_left_box;
    ImageFloatBox m_right_box;
};

}
}
}

#endif
