/*  Prize Select Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_PrizeSelectDetector_H
#define PokemonAutomation_PokemonFRLG_PrizeSelectDetector_H

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

// Same as WhiteDialogDetector, but also looks for the white on the final line of the prize select box
// All 3 prize booths have 5 items, however the width of the box varies so we have to go by height
// (also I'm not sure if width varies by language)
// can't use the coin count in the top left, as that appears as soon as you talk to the booth
// a future improvement might be to look for the selection arrow, but keep in mind width varies
class PrizeSelectDetector : public StaticScreenDetector{
public:
    PrizeSelectDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_right_box;
    ImageFloatBox m_top_box;
    ImageFloatBox m_bottom_box;
    ImageFloatBox m_selection_box;
};
class PrizeSelectWatcher : public DetectorToFinder<PrizeSelectDetector>{
public:
    PrizeSelectWatcher(Color color)
        : DetectorToFinder("PrizeSelectWatcher", std::chrono::milliseconds(250), color)
    {}
};




}
}
}

#endif
