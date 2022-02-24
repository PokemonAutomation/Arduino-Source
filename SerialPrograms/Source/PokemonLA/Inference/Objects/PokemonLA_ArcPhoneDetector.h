/*  Arc Phone Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_ArcPhoneDetector_H
#define PokemonAutomation_PokemonLA_ArcPhoneDetector_H

#include "Common/Cpp/SpinLock.h"
#include "Common/Cpp/Color.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"
#include "PokemonLA/Inference/Objects/PokemonLA_WhiteObjectDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class ArcPhoneMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    ArcPhoneMatcher();
    static const ArcPhoneMatcher& instance();
};



class ArcPhoneTracker : public WhiteObjectDetector{
public:
    ArcPhoneTracker();

    virtual void process_object(const QImage& image, const WaterfillObject& object) override;
    virtual void finish() override;
};



class ArcPhoneDetector : public VisualInferenceCallback{
public:
    ArcPhoneDetector(
        Logger& logger, VideoOverlay& overlay,
        std::chrono::milliseconds min_streak,
        bool stop_on_detected
    );

    bool detected() const{
        return m_detected.load(std::memory_order_acquire);
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;

private:
    Logger& m_logger;
    ImageFloatBox m_box;
    std::chrono::milliseconds m_min_streak;
    bool m_stop_on_detected;

    SpinLock m_lock;
    ArcPhoneTracker m_tracker;
    WhiteObjectWatcher m_watcher;

    std::chrono::system_clock::time_point m_last_flip;
    bool m_current_streak;

    std::atomic<bool> m_detected;
};



}
}
}
#endif
