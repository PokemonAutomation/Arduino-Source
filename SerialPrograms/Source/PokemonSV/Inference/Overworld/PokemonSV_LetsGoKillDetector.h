/*  Let's Go Kill Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_LetsGoKillDetector_H
#define PokemonAutomation_PokemonSV_LetsGoKillDetector_H

#include <functional>
#include <atomic>
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
        std::function<void()> on_kill_callback = nullptr,
        std::chrono::milliseconds duration = std::chrono::milliseconds(250)
    );

    WallClock last_kill() const{
        return m_last_detected.load(std::memory_order_relaxed);
    }

    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    Logger& m_logger;
    bool m_trigger_if_detected;
    std::function<void()> m_on_kill_callback;

    bool m_last_detection;
    std::atomic<WallClock> m_last_detected;
};




}
}
}
#endif
