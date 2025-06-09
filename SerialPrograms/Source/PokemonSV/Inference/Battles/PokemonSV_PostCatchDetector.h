/*  Post-Catch Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_PostCatchDetector_H
#define PokemonAutomation_PokemonSV_PostCatchDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class AddToPartyDetector : public StaticScreenDetector{
public:
    AddToPartyDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

protected:
    Color m_color;
    ImageFloatBox m_top_left;
    ImageFloatBox m_top_right;
    ImageFloatBox m_bottom_left;
    ImageFloatBox m_bottom_right;
    PromptDialogDetector m_dialog;
};
class AddToPartyWatcher : public DetectorToFinder<AddToPartyDetector>{
public:
    AddToPartyWatcher(Color color = COLOR_RED)
         : DetectorToFinder("AddToPartyFinder", std::chrono::milliseconds(250), color)
    {}
};






}
}
}
#endif
