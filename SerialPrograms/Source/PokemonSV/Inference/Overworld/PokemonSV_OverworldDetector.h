/*  Overworld Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_OverworldDetector_H
#define PokemonAutomation_PokemonSV_OverworldDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

// Detect the player character is on overworld.
// It detects the center circle of the minimap.
class OverworldDetector : public StaticScreenDetector{
public:
    OverworldDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    // return coordinates of the radar ball. return -1, -1 if unable to find the radar ball.
    // set strict_requirements to true for more accurate detection of radar ball,
    std::pair<double, double> locate_ball(const ImageViewRGB32& screen, bool strict_requirements) const;

protected:
    const Color m_color;
    const ImageFloatBox m_ball;
    const ImageFloatBox m_radar;
    const ImageFloatBox m_radar_inside;
};

class OverworldWatcher : public OverworldDetector, public VisualInferenceCallback{
public:
    OverworldWatcher(Color color = COLOR_RED, bool detect_event = false);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const VideoSnapshot& frame) override;


private:
    std::chrono::milliseconds m_ball_hold_duration;
    std::chrono::milliseconds m_map_hold_duration;
    WallClock m_last_ball;
    VideoSnapshot m_start_of_detection;
    bool m_detect_event;
};


}
}
}
#endif
