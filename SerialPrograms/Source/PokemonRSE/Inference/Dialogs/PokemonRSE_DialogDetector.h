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

// Future note: when given a choice popup, there is no advance arrow.


}
}
}

#endif
