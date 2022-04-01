/*  Arc Phone Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_ArcPhoneDetector_H
#define PokemonAutomation_PokemonLA_ArcPhoneDetector_H

#include "Common/Cpp/SpinLock.h"
#include "Common/Cpp/Color.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonFramework/Inference/DetectionDebouncer.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "PokemonLA_WhiteObjectDetector.h"
#include "PokemonLA_ButtonDetector.h"

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

    virtual void process_object(const ConstImageRef& image, const WaterfillObject& object) override;
    virtual void finish() override;
};



class ArcPhoneDetector : public VisualInferenceCallback{
public:
    ArcPhoneDetector(
        LoggerQt& logger, VideoOverlay& overlay,
        std::chrono::milliseconds min_streak,
        bool stop_on_detected
    );

    bool detected() const{
        return m_debouncer_phone.get() && m_debouncer_button.get();
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;

private:
    LoggerQt& m_logger;
    ImageFloatBox m_box;
    bool m_stop_on_detected;

    SpinLock m_lock;
    ArcPhoneTracker m_tracker_phone;
    ButtonTracker m_tracker_button;
    WhiteObjectWatcher m_watcher;

    DetectionDebouncer<bool> m_debouncer_phone;
    DetectionDebouncer<bool> m_debouncer_button;
};



}
}
}
#endif
