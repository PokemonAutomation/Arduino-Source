/*  Map Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_MapDetector_H
#define PokemonAutomation_PokemonFRLG_MapDetector_H

#include <chrono>
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

// Detect the default (first) Pokemon Summary page
// This has nature and OT, but no stats
// This will work with all languages, as the page indicators are in the same positions
class KantoMapDetector : public StaticScreenDetector{
public:
    KantoMapDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_top_box;
    ImageFloatBox m_left_box;
    ImageFloatBox m_right_box;
    ImageFloatBox m_land_box;
};
class KantoMapWatcher : public DetectorToFinder<KantoMapDetector>{
public:
    KantoMapWatcher(Color color)
        : DetectorToFinder("KantoMapWatcher", std::chrono::milliseconds(250), color)
    {}
};



}
}
}

#endif
