/*  Stationary Overworld Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_StationaryOverworldWatcher_H
#define PokemonAutomation_PokemonSV_StationaryOverworldWatcher_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class StationaryOverworldWatcher : public VisualInferenceCallback{
public:
    StationaryOverworldWatcher(Color color = COLOR_RED, ImageFloatBox box = {0.865, 0.82, 0.08, 0.1}, size_t seconds_stationary = 5);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    // return true if the screen within `m_box` doesn't change for am amount of time as per `seconds_stationary`
    // the overworld must be detected throughout this.
    virtual bool process_frame(const VideoSnapshot& frame) override;


private:
    const Color m_color;
    const ImageFloatBox m_box;
    OverworldDetector m_overworld_detector;
    const std::chrono::milliseconds m_map_hold_duration;
    VideoSnapshot m_snapshot_start;
};


}
}
}
#endif
