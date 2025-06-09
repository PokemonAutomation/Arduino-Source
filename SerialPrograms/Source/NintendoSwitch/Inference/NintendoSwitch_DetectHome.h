/*  Detect Home
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 *      This file is in the process of being split up and refactored.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_DetectHome_H
#define PokemonAutomation_NintendoSwitch_DetectHome_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{







//  Detect the "Checking if the software can be played..." menu.
class CheckOnlineDetector : public StaticScreenDetector{
public:
    CheckOnlineDetector(Color color = COLOR_RED, bool invert = false);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    bool m_invert;
    ImageFloatBox m_box_top;
    ImageFloatBox m_box_mid;
    ImageFloatBox m_top;
    ImageFloatBox m_left;
    ImageFloatBox m_bottom_solid;
    ImageFloatBox m_bottom_buttons;
};
class CheckOnlineWatcher : public DetectorToFinder<CheckOnlineDetector>{
public:
    CheckOnlineWatcher(Color color = COLOR_RED, bool invert = false)
         : DetectorToFinder("CheckOnlineWatcher", std::chrono::milliseconds(250), color, invert)
    {}
};


}
}
#endif

