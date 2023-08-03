/*  Pokedex Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_PokedexMenuDetector_H
#define PokemonAutomation_PokemonSV_PokedexMenuDetector_H

#include <functional>
#include <atomic>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Inference/VisualDetector.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonSV{



class PokedexMenuDetector : public StaticScreenDetector{
public:
    PokedexMenuDetector(Color color, const ImageFloatBox& box = {0.23, 0.23, 0.04, 0.08});

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    ImageFloatBox m_box;
};
class PokedexMenuWatcher : public DetectorToFinder<PokedexMenuDetector>{
public:
    PokedexMenuWatcher(
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
