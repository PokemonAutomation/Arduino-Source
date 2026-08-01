/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonRSE_DialogDetector_H
#define PokemonAutomation_PokemonRSE_DialogDetector_H

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
namespace PokemonRSE{


// Detect the Pokenav dialog box that appears when being called on the overworld
// This does not work for calls from the Pokenav menu itself, as the position is different
// Emerald only, Ruby and Sapphire don't have to worry about randomly being interrupted.
// Advance arrow appears on all but the last box of dialog. No detection for this since
// most interaction will be to mash B to get rid of the call.
class PokenavDialogDetector : public StaticScreenDetector{
public:
    PokenavDialogDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_dialog_border_box;
    ImageFloatBox m_dialog_main_box;
};
class PokenavDialogWatcher : public DetectorToFinder<PokenavDialogDetector>{
public:
    PokenavDialogWatcher(Color color)
        : DetectorToFinder("PokenavDialogWatcher", std::chrono::milliseconds(250), color)
    {}
};


// Dialog box without advance arrow.
// Positions are different between japan and ROW, but are the same across games.
class DialogDetector : public StaticScreenDetector{
public:
    DialogDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    //Position for non-JPN
    ImageFloatBox m_top_box;
    ImageFloatBox m_inner_box;

    //Position for JPN
    ImageFloatBox m_side_box_jpn;
    ImageFloatBox m_inner_box_jpn;
};
class DialogWatcher : public DetectorToFinder<DialogDetector>{
public:
    DialogWatcher(Color color)
        : DetectorToFinder("DialogWatcher", std::chrono::milliseconds(250), color)
    {}
};



// Dialog box, now with arrow!
class AdvanceDialogDetector : public StaticScreenDetector{
public:
    AdvanceDialogDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    //Position for non-JPN
    ImageFloatBox m_top_box;
    ImageFloatBox m_inner_box;
    ImageFloatBox m_dialog_box;

    //Position for JPN
    ImageFloatBox m_side_box_jpn;
    ImageFloatBox m_inner_box_jpn;
    ImageFloatBox m_dialog_jpn_box;
};
class AdvanceDialogWatcher : public DetectorToFinder<AdvanceDialogDetector>{
public:
    AdvanceDialogWatcher(Color color)
        : DetectorToFinder("AdvanceDialogWatcher", std::chrono::milliseconds(250), color)
    {}
};



// Detection for the Yes/No prompt
class SelectionDialogDetector : public StaticScreenDetector{
public:
    SelectionDialogDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    //Position for non-JPN
    ImageFloatBox m_top_box;
    ImageFloatBox m_inner_box;

    //Position for JPN
    ImageFloatBox m_side_box_jpn;
    ImageFloatBox m_inner_box_jpn;

    ImageFloatBox m_yes_box;
    ImageFloatBox m_no_box;
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
