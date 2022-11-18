/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_DialogDetector_H
#define PokemonAutomation_PokemonSV_DialogDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



//  Detect dialog that has the small arrow at bottom to show the next dialog.
class AdvanceDialogDetector : public StaticScreenDetector{
public:
    AdvanceDialogDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

protected:
    Color m_color;
    ImageFloatBox m_box_top;
    ImageFloatBox m_box_bot;
    ImageFloatBox m_arrow;
};
class AdvanceDialogFinder : public DetectorToFinder_ConsecutiveDebounce<AdvanceDialogDetector>{
public:
    AdvanceDialogFinder(Color color = COLOR_RED)
         : DetectorToFinder_ConsecutiveDebounce("AdvanceDialogFinder", 5, color)
    {}
};



//  Detect dialog that prompts the player to make a choice.
class PromptDialogDetector : public StaticScreenDetector{
public:
    PromptDialogDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

protected:
    Color m_color;
    ImageFloatBox m_box_top;
    ImageFloatBox m_box_bot;
    ImageFloatBox m_gradient;
};
class PromptDialogFinder : public DetectorToFinder_ConsecutiveDebounce<PromptDialogDetector>{
public:
    PromptDialogFinder(Color color = COLOR_RED)
         : DetectorToFinder_ConsecutiveDebounce("PromptDialogFinder", 5, color)
    {}
};





}
}
}
#endif
