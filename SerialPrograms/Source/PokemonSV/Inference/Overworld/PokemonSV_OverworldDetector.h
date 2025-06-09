/*  Overworld Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_OverworldDetector_H
#define PokemonAutomation_PokemonSV_OverworldDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

// Detect the player character is on overworld.
// It detects the center circle of the minimap.
class OverworldDetector : public StaticScreenDetector{
public:
    OverworldDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

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
    OverworldWatcher(Logger& logger, Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const VideoSnapshot& frame) override;


private:
    Logger& m_logger;
    std::chrono::milliseconds m_ball_hold_duration;
    std::chrono::milliseconds m_map_hold_duration;
    std::chrono::milliseconds m_north_hold_duration;
    WallClock m_last_ball;
    WallClock m_last_north;
    VideoSnapshot m_start_of_detection;
    DirectionDetector m_direction_detector;
};


}
}
}
#endif
