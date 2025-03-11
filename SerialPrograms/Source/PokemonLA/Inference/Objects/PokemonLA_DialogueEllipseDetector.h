/*  Dialogue Ellipse Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Detect the white wide ellipse at the end of the transparent dialogue box.
 *  The transparent dialogue box comes from your opponent shown before and/or after the battle, like
 *  Ingo's battles or Fortune Sisters'.
 */

#ifndef PokemonAutomation_PokemonLA_DialogueEllipseDetector_H
#define PokemonAutomation_PokemonLA_DialogueEllipseDetector_H

#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/DetectionDebouncer.h"
#include "PokemonLA/Inference/Objects/PokemonLA_WhiteObjectDetector.h"

namespace PokemonAutomation{
    class Logger;
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

    virtual void process_object(const ImageViewRGB32& image, const WaterfillObject& object) override;
    virtual void finish(const ImageViewRGB32& image) override;
};



class DialogueEllipseDetector : public VisualInferenceCallback{
public:
    DialogueEllipseDetector(
        Logger& logger, VideoOverlay& overlay,
        std::chrono::milliseconds min_streak,
        bool stop_on_detected
    );

    bool detected() const{
        return m_debouncer.get();
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    Logger& m_logger;
    ImageFloatBox m_box;
    bool m_stop_on_detected;

    DialogueEllipseTracker m_tracker;
    WhiteObjectWatcher m_watcher;

    DetectionDebouncer<bool> m_debouncer;
};



}
}
}
#endif
