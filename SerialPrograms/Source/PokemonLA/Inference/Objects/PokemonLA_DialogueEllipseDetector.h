/*  Dialogue Ellipse Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Detect the white wide ellipse at the end of the transparent dialogue box.
 */

#ifndef PokemonAutomation_PokemonLA_DialogueEllipseDetector_H
#define PokemonAutomation_PokemonLA_DialogueEllipseDetector_H

#include "Common/Cpp/SpinLock.h"
#include "Common/Cpp/Color.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonFramework/Inference/DetectionDebouncer.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "PokemonLA/Inference/Objects/PokemonLA_WhiteObjectDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class DialogueEllipseMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    DialogueEllipseMatcher();
    static const DialogueEllipseMatcher& instance();
};



class DialogueEllipseTracker : public WhiteObjectDetector{
public:
    DialogueEllipseTracker();

    virtual void process_object(const ConstImageRef& image, const WaterfillObject& object) override;
    virtual void finish() override;
};



class DialogueEllipseDetector : public VisualInferenceCallback{
public:
    DialogueEllipseDetector(
        LoggerQt& logger, VideoOverlay& overlay,
        std::chrono::milliseconds min_streak,
        bool stop_on_detected
    );

    bool detected() const{
        return m_debouncer.get();
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool process_frame(const QImage& frame, WallClock timestamp) override;

private:
    LoggerQt& m_logger;
    ImageFloatBox m_box;
    bool m_stop_on_detected;

    SpinLock m_lock;
    DialogueEllipseTracker m_tracker;
    WhiteObjectWatcher m_watcher;

    DetectionDebouncer<bool> m_debouncer;
};



}
}
}
#endif
