/*  Start Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_StartMenuDetector_H
#define PokemonAutomation_PokemonFRLG_StartMenuDetector_H

#include <chrono>
#include <atomic>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
    class CancellableScope;
    class VideoFeed;
namespace NintendoSwitch{
namespace PokemonFRLG{

// Detect Start menu by looking for the blue info panel
class StartMenuDetector : public StaticScreenDetector{
public:
    StartMenuDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_right_box;
    ImageFloatBox m_top_box;
    ImageFloatBox m_bottom_box;
};
class StartMenuWatcher : public DetectorToFinder<StartMenuDetector>{
public:
    StartMenuWatcher(Color color)
        : DetectorToFinder("StartMenuWatcher", std::chrono::milliseconds(250), color)
    {}
};


}
}
}

#endif
