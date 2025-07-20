/*  Battle Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLGPE_BattleArrowDetector_H
#define PokemonAutomation_PokemonLGPE_BattleArrowDetector_H

#include <chrono>
#include <atomic>
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
class BattleArrowWatcher : public DetectorToFinder<BattleArrowDetector>{
public:
    BattleArrowWatcher(Color color = COLOR_RED, ImageFloatBox box = {0.546, 0.863, 0.045, 0.068})
         : DetectorToFinder("BattleArrowWatcher", std::chrono::milliseconds(100), color, box)
    {}
};



}
}
}

#endif
