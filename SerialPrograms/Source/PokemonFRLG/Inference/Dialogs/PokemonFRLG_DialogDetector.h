/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_DialogDetector_H
#define PokemonAutomation_PokemonFRLG_DialogDetector_H

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

// When given a choice popup, there is no advance arrow.
// FRLG doesn't have an advance arrow on the final line of dialog when speaking to an npc!

//Standard - npc dialog, interacting with items, etc.
//tutorial - white box with grey mat like a picture frame

// Common dialog box for npcs, etc.
// This one detects the final line spoken, so no red arrow
// This also detects all the other white dialogs, so use sparingly
class WhiteDialogDetector : public StaticScreenDetector{
public:
    WhiteDialogDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_right_box;
    ImageFloatBox m_top_box;
    ImageFloatBox m_bottom_box;
};
class WhiteDialogWatcher : public DetectorToFinder<WhiteDialogDetector>{
public:
    WhiteDialogWatcher(Color color)
        : DetectorToFinder("WhiteDialogWatcher", std::chrono::milliseconds(250), color)
    {}
};

// Same as WhiteDialogDetector, but filter for the red arrow
// Detect the red advancement arrow by filtering for DARK red.
// There is red/pink color text for female npcs in non-japan versions
// might be an issue if using it for things other than shiny-hunt resetting
// all gifts are black/blue dialogs?
class AdvanceWhiteDialogDetector : public StaticScreenDetector{
public:
    AdvanceWhiteDialogDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_dialog_box;
    ImageFloatBox m_right_box;
    ImageFloatBox m_top_box;
    ImageFloatBox m_bottom_box;
};
class AdvanceWhiteDialogWatcher : public DetectorToFinder<AdvanceWhiteDialogDetector>{
public:
    AdvanceWhiteDialogWatcher(Color color)
        : DetectorToFinder("AdvanceWhiteDialogWatcher", std::chrono::milliseconds(250), color)
    {}
};

// Same as WhiteDialogDetector, but also looks for the white of the Yes/No box
class SelectionDialogDetector : public StaticScreenDetector{
public:
    SelectionDialogDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_right_box;
    ImageFloatBox m_top_box;
    ImageFloatBox m_bottom_box;
    ImageFloatBox m_selection_box;
};
class SelectionDialogWatcher : public DetectorToFinder<SelectionDialogDetector>{
public:
    SelectionDialogWatcher(Color color)
        : DetectorToFinder("SelectionDialogWatcher", std::chrono::milliseconds(250), color)
    {}
};




}
}
}

#endif
