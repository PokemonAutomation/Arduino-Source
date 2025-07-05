/*  Let's Go Kill Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_LetsGoKillDetector_H
#define PokemonAutomation_PokemonSV_LetsGoKillDetector_H

#include <functional>
#include <atomic>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/Audio/AudioPerSpectrumDetectorBase.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonSV{



class LetsGoKillDetector : public StaticScreenDetector{
public:
    LetsGoKillDetector(Color color, const ImageFloatBox& box = {0.71, 0.15, 0.04, 0.08});

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_box;
};
class LetsGoKillWatcher : public DetectorToFinder<LetsGoKillDetector>{
public:
    LetsGoKillWatcher(
        Logger& logger,
        Color color, bool trigger_if_detected = true,
        const ImageFloatBox& box = {0.71, 0.15, 0.04, 0.08},
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




class LetsGoKillSoundDetector : public AudioPerSpectrumDetectorBase{
public:
    LetsGoKillSoundDetector(Logger& logger, DetectedCallback detected_callback = nullptr);

    virtual float get_score_threshold() const override;

    WallClock last_kill() const{
        return m_last_detected.load(std::memory_order_relaxed);
    }

private:
    virtual std::unique_ptr<SpectrogramMatcher> build_spectrogram_matcher(size_t sample_rate) override;

private:
    std::atomic<WallClock> m_last_detected;
};



}
}
}
#endif
