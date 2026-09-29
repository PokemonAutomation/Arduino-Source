/*  Load Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_LoadMenuDetector_H
#define PokemonAutomation_PokemonFRLG_LoadMenuDetector_H

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

// Detect save load menu by looking for the blue on the sides and the white of the save file
// Untested on new game, assumes there's a save.
class LoadMenuDetector : public StaticScreenDetector{
public:
    LoadMenuDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_right_box;
    ImageFloatBox m_left_box;
    ImageFloatBox m_save_box;
};
class LoadMenuWatcher : public DetectorToFinder<LoadMenuDetector>{
public:
    LoadMenuWatcher(Color color)
        : DetectorToFinder("LoadMenuWatcher", std::chrono::milliseconds(250), color)
    {}
};


}
}
}

#endif
