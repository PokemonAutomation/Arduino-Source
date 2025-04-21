/*  Battle Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLGPE_BattleMenuDetector_H
#define PokemonAutomation_PokemonLGPE_BattleMenuDetector_H

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
namespace PokemonLGPE{

//Detect the light blue of your HP box and your opponent's HP box
class BattleMenuDetector : public StaticScreenDetector{
public:
    BattleMenuDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    ImageFloatBox m_your_box;
    ImageFloatBox m_opponent_box;
};
class BattleMenuWatcher : public DetectorToFinder<BattleMenuDetector>{
public:
    BattleMenuWatcher(Color color)
        : DetectorToFinder("BattleMenuWatcher", std::chrono::milliseconds(250), color)
    {}
};

//Battle dialog detector - white of the box

/*
Possible future improvements:
Detect arrow and position
|Fight|Pokemon|Bag|Run| for wild battle
|---|Fight|Pokemon|Bag| for trainer battle
(align right)

Detect Y Info button located next to menu.
Don't know what changes for a double battle.
*/

}
}
}

#endif
