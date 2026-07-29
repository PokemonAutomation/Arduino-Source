/*  Box View Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_BoxViewDetector_H
#define PokemonAutomation_PokemonHome_BoxViewDetector_H

#include <optional>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "PokemonHome/Inference/PokemonHome_ButtonDetector.h"

namespace PokemonAutomation{
class Logger;
namespace NintendoSwitch{
namespace PokemonHome{

class BoxViewDetector : public StaticScreenDetector{
public:
    BoxViewDetector(VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    virtual void reset_state() override { m_button_plus_detector.reset_state(); }

private:
    ButtonDetector m_button_plus_detector;

};
class BoxViewWatcher : public DetectorToFinder<BoxViewDetector>{
public:
    BoxViewWatcher(VideoOverlay* overlay = nullptr)
        : DetectorToFinder("BoxViewWatcher", Milliseconds(100), overlay)
    {}
};


}
}
}
#endif
