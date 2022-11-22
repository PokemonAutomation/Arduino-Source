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
class AdvanceDialogHold : public DetectorToFinder_HoldDebounce<AdvanceDialogDetector>{
public:
    AdvanceDialogHold(std::chrono::milliseconds duration, Color color = COLOR_RED)
         : DetectorToFinder_HoldDebounce("AdvanceDialogHold", duration, color)
    {}
};



//  Detect dialog that prompts the player to make a choice.
class PromptDialogDetector : public StaticScreenDetector{
public:
    //  Will catch any prompt.
    PromptDialogDetector(Color color = COLOR_RED);

    //  Will only look for prompts with the cursor in this box.
    PromptDialogDetector(const ImageFloatBox& arrow_box, Color color = COLOR_RED);

    //  Will only catch prompts with the cursor at a specific line.
    //  (line_index = 0) is the lowest line.
    //  (line_index = 1) is the next lowest line.
    PromptDialogDetector(size_t line_index, Color color = COLOR_RED);

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
    PromptDialogFinder(const ImageFloatBox& arrow_box, Color color = COLOR_RED)
         : DetectorToFinder_ConsecutiveDebounce("PromptDialogFinder", 5, arrow_box, color)
    {}
    PromptDialogFinder(size_t line_index, Color color = COLOR_RED)
         : DetectorToFinder_ConsecutiveDebounce("PromptDialogFinder", 5, line_index, color)
    {}
};





}
}
}
#endif
