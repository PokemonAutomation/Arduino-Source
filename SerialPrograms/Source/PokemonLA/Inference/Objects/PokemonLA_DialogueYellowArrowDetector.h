/*  Dialogue Yellow Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  The yellow marker pointing to the button to advance dialogue box.
 *  For different types of dialogue boxes, there are different types of yellow arrows.
 *  Currently we only need to detect the yellow arrows for transparent dialogue boxes happened
 *  during Ingo's battles.
 */

#ifndef PokemonAutomation_PokemonLA_DialogueYellowArrowDetector_H
#define PokemonAutomation_PokemonLA_DialogueYellowArrowDetector_H

#include "Common/Cpp/SpinLock.h"
#include "Common/Cpp/Color.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonFramework/Inference/DetectionDebouncer.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "PokemonLA_WhiteObjectDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class DialogueYellowArrowMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    DialogueYellowArrowMatcher();
    static const DialogueYellowArrowMatcher& instance();
};

class DialogueYellowArrowTracker : public WhiteObjectDetector{
public:
    DialogueYellowArrowTracker();

    virtual void process_object(const ConstImageRef& image, const WaterfillObject& object) override;
    virtual void finish() override;
};


class DialogueYellowArrowDetector : public VisualInferenceCallback{
public:
    DialogueYellowArrowDetector(
        LoggerQt& logger, VideoOverlay& overlay,
        bool stop_on_detected
    );

    bool detected() const{
        return m_detected.load(std::memory_order_acquire);
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool process_frame(const QImage& frame, WallClock timestamp) override;

private:
    LoggerQt& m_logger;
    ImageFloatBox m_box;
    bool m_stop_on_detected;

    DialogueYellowArrowTracker m_tracker;
    WhiteObjectWatcher m_watcher;

    std::atomic<bool> m_detected;
};



}
}
}
#endif
