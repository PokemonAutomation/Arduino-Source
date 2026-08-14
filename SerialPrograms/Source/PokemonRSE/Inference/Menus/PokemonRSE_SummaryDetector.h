/*  Summary Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonRSE_SummaryDetector_H
#define PokemonAutomation_PokemonRSE_SummaryDetector_H

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
namespace PokemonRSE{

// Detect the default (first) Pokemon Summary page
// This has nature and OT, but no stats
// This will work with all languages, as the page indicators are in the same positions
// pg 2 stats, pg 3 moves, pg4 contest
class SummaryDetector : public StaticScreenDetector{
public:
    SummaryDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_top_bar_box;

    ImageFloatBox m_page_1_box;
    ImageFloatBox m_page_2_box;
    ImageFloatBox m_page_3_box;
    ImageFloatBox m_page_4_box;
};
class SummaryWatcher : public DetectorToFinder<SummaryDetector>{
public:
    SummaryWatcher(Color color)
        : DetectorToFinder("SummaryWatcher", std::chrono::milliseconds(250), color)
    {}
};

}
}
}

#endif
