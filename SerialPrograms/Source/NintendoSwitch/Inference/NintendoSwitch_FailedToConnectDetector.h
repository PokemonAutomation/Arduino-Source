/*  Failed To Connect Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_FailedToConnectDetector_H
#define PokemonAutomation_NintendoSwitch_FailedToConnectDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



//  Detect the "Failed to connect to internet" menu that appears after
// "Checking if the software can be played..." failed
class FailedToConnectDetector : public StaticScreenDetector{
public:
    FailedToConnectDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_close_button;
    ImageFloatBox m_top;
    ImageFloatBox m_bottom;
};
class FailedToConnectWatcher : public DetectorToFinder<FailedToConnectDetector>{
public:
    FailedToConnectWatcher(Color color = COLOR_RED)
         : DetectorToFinder("FailedToConnectWatcher", std::chrono::milliseconds(250), color)
    {}
};



}
}
#endif

