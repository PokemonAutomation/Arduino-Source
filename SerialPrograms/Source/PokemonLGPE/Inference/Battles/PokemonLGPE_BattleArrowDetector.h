/*  Battle Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLGPE_BattleArrowDetector_H
#define PokemonAutomation_PokemonLGPE_BattleArrowDetector_H

#include <chrono>
//#include <atomic>
#include <deque>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

class BattleArrowDetector : public StaticScreenDetector{
public:
    /*
    |Fight|Pokemon|Bag|Run| for wild battle
    |---|Fight|Pokemon|Bag| for trainer battle
    (align right)
    Don't know what changes for a double battle.
    */
    //Arrow bounces back and forth.
    BattleArrowDetector(Color color, const ImageFloatBox& box);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

protected:
    Color m_color;
    ImageFloatBox m_box;
};


//
//  We use a custom watcher because the arrow wiggles and thus is less likely to
//  maintain consecutive detections.
//
class BattleArrowWatcher : public BattleArrowDetector, public VisualInferenceCallback{
public:
    BattleArrowWatcher(Color color = COLOR_RED, ImageFloatBox box = {0.546, 0.863, 0.045, 0.068})
         : BattleArrowDetector(color, box)
         , VisualInferenceCallback("BattleArrowWatcher")
    {}

    virtual void make_overlays(VideoOverlaySet& items) const override{
        BattleArrowDetector::make_overlays(items);
    }

    using VisualInferenceCallback::process_frame;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    std::deque<WallClock> m_detections;
};



}
}
}

#endif
