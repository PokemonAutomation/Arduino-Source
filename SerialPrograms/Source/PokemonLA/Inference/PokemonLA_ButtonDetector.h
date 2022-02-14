/*  Button Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_ButtonDetector_H
#define PokemonAutomation_PokemonLA_ButtonDetector_H

#include "Common/Cpp/SpinLock.h"
#include "Common/Cpp/Color.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"
#include "PokemonLA/Inference/PokemonLA_WhiteObjectDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


enum class ButtonType{
    ButtonA,
    ButtonB,
};


class ButtonMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:

public:
    ButtonMatcher(ButtonType type);
    static const ButtonMatcher& A();
    static const ButtonMatcher& B();
};




class ButtonTracker : public WhiteObjectDetector{
public:
    ButtonTracker(ButtonType type);

    virtual void process_object(const QImage& image, const WaterfillObject& object) override;
    virtual void finish() override;

private:
    const ButtonMatcher& m_matcher;
};



class ButtonDetector : public VisualInferenceCallback{
public:
    ButtonDetector(
        Logger& logger, VideoOverlay& overlay,
        ButtonType type,
        const ImageFloatBox& box
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

    SpinLock m_lock;
    ButtonTracker m_tracker;
    WhiteObjectWatcher m_watcher;

    std::chrono::system_clock::time_point m_last_flip;
    bool m_current_streak;

    std::atomic<bool> m_detected;
};



}
}
}
#endif
