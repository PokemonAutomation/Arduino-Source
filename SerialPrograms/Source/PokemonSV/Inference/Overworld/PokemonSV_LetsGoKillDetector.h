/*  Let's Go Kill Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_LetsGoKillDetector_H
#define PokemonAutomation_PokemonSV_LetsGoKillDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Inference/VisualDetector.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonSV{



class LetsGoKillDetector : public StaticScreenDetector{
public:
    LetsGoKillDetector(Color color, const ImageFloatBox& box = {0.71, 0.15, 0.04, 0.30});

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    ImageFloatBox m_box;
};
class LetsGoKillWatcher : public DetectorToFinder<LetsGoKillDetector>{
public:
    LetsGoKillWatcher(
        Logger& logger,
        Color color, bool trigger_if_detected,
        std::chrono::milliseconds duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("LetsGoKillWatcher", duration, color)
         , m_logger(logger)
         , m_trigger_if_detected(trigger_if_detected)
    {}

    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    Logger& m_logger;
    bool m_trigger_if_detected;
    bool m_last_detected = false;
};




}
}
}
#endif
