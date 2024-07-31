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


enum class DialogType{
    DIALOG_WHITE,
    DIALOG_BLACK,
    DIALOG_ALL
};

//  Detect any dialog box.
class DialogBoxDetector : public StaticScreenDetector{
public:
    DialogBoxDetector(Color color = COLOR_RED, bool true_if_detected = true, DialogType type = DialogType::DIALOG_ALL);

    Color color() const{ return m_color; }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    bool m_true_if_detected;
    ImageFloatBox m_box_top;
    ImageFloatBox m_box_bot;
    ImageFloatBox m_border_top;
    ImageFloatBox m_border_bot;
    DialogType m_dialog_type;
};
class DialogBoxWatcher : public DetectorToFinder<DialogBoxDetector>{
public:
    DialogBoxWatcher(
        Color color,
        bool trigger_if_detected,
        std::chrono::milliseconds duration = std::chrono::milliseconds(250),
        DialogType type = DialogType::DIALOG_ALL
    )
         : DetectorToFinder("DialogBoxWatcher", duration, color, trigger_if_detected, type)
    {}
};




//  Detect dialog that has the small arrow at bottom to show the next dialog.
//  It should be able to detect both the white background dialog and black background dialog.
class AdvanceDialogDetector : public StaticScreenDetector{
public:
    AdvanceDialogDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    DialogBoxDetector m_box;
    ImageFloatBox m_arrow;
};
class AdvanceDialogWatcher : public DetectorToFinder<AdvanceDialogDetector>{
public:
    AdvanceDialogWatcher(Color color, std::chrono::milliseconds duration = std::chrono::milliseconds(250))
         : DetectorToFinder("AdvanceDialogWatcher", duration, color)
    {}
};



//  Detect dialog that prompts the player to make a choice.
//  i.e. detects that Dialog box and Gradient arrow are present
class PromptDialogDetector : public StaticScreenDetector{
public:
    //  Will catch any prompt.
    PromptDialogDetector(Color color);

    //  Will only look for prompts with the cursor in this box.
    PromptDialogDetector(Color color, const ImageFloatBox& arrow_box);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    DialogBoxDetector m_box;
    ImageFloatBox m_gradient;
};
class PromptDialogWatcher : public DetectorToFinder<PromptDialogDetector>{
public:
    PromptDialogWatcher(
        Color color,
        std::chrono::milliseconds duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("PromptDialogWatcher", std::chrono::milliseconds(250), color)
    {}
    PromptDialogWatcher(
        Color color,
        const ImageFloatBox& arrow_box,
        std::chrono::milliseconds duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("PromptDialogWatcher", std::chrono::milliseconds(250), color, arrow_box)
    {}
};





}
}
}
#endif
