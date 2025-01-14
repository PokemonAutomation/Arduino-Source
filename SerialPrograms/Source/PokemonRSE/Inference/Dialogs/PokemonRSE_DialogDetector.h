/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonRSE_DialogDetector_H
#define PokemonAutomation_PokemonRSE_DialogDetector_H

#include <chrono>
#include <atomic>
//#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"

namespace PokemonAutomation{
    class CancellableScope;
    class VideoFeed;
namespace NintendoSwitch{
namespace PokemonRSE{

/*
// Detect that a dialog box is on screen by looking for the white of the box
class DialogDetector : public StaticScreenDetector{
public:
    DialogDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    ImageFloatBox m_left_box;
    ImageFloatBox m_right_box;
};
class DialogWatcher : public DetectorToFinder<DialogDetector>{
public:
    DialogWatcher(Color color)
        : DetectorToFinder("DialogWatcher", std::chrono::milliseconds(250), color)
    {}
};
*/

// Battle dialog boxes are teal
class BattleDialogDetector : public StaticScreenDetector{
public:
    BattleDialogDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    ImageFloatBox m_left_box;
    ImageFloatBox m_right_box;
};
class BattleDialogWatcher : public DetectorToFinder<BattleDialogDetector>{
public:
    BattleDialogWatcher(Color color)
        : DetectorToFinder("BattleDialogWatcher", std::chrono::milliseconds(250), color)
    {}
};


// Battle menu is up when it is white on the right and teal on the left
class BattleMenuDetector : public StaticScreenDetector{
public:
    BattleMenuDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    ImageFloatBox m_left_box;
    ImageFloatBox m_right_box;
};
class BattleMenuWatcher : public DetectorToFinder<BattleMenuDetector>{
public:
    BattleMenuWatcher(Color color)
        : DetectorToFinder("BattleMenuWatcher", std::chrono::milliseconds(250), color)
    {}
};



// Detect the red advancement arrow by filtering for red.
// This works for now, I don't think there's colored text?
// TODO: Change this to detect that the dialog arrow is in the dialog box by filtering for the red arrow
class AdvanceDialogDetector : public StaticScreenDetector{
public:
    AdvanceDialogDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    ImageFloatBox m_dialog_box;
};
class AdvanceDialogWatcher : public DetectorToFinder<AdvanceDialogDetector>{
public:
    AdvanceDialogWatcher(Color color)
        : DetectorToFinder("AdvanceDialogWatcher", std::chrono::milliseconds(250), color)
    {}
};

// when given a choice popup, there is no advance arrow


}
}
}

#endif
