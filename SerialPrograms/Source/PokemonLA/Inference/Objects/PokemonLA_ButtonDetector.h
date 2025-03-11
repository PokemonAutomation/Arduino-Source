/*  Button Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_ButtonDetector_H
#define PokemonAutomation_PokemonLA_ButtonDetector_H

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/DetectionDebouncer.h"
#include "PokemonLA/Inference/Objects/PokemonLA_WhiteObjectDetector.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonLA{


enum class ButtonType{
    ButtonA,
    ButtonB,
    ButtonPlus,
    ButtonMinus,
    ArrowLeft,
    ArrowRight,
};


class ButtonMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    ButtonMatcher(ButtonType type, size_t min_width, size_t max_width, double max_rmsd);
    static const ButtonMatcher& A();
    static const ButtonMatcher& B();
    static const ButtonMatcher& Plus();
    static const ButtonMatcher& Minus();
    static const ButtonMatcher& ArrowLeft();
    static const ButtonMatcher& ArrowRight();

    virtual bool check_image(const ImageViewRGB32& image) const override{
        return image.width() >= m_min_width && image.height() >= m_min_height;
    };

    size_t m_min_width;
    size_t m_min_height;
    double m_max_rmsd;
};




class ButtonTracker : public WhiteObjectDetector{
public:
    ButtonTracker(ButtonType type);

    virtual void process_object(const ImageViewRGB32& image, const WaterfillObject& object) override;
    virtual void finish(const ImageViewRGB32& image) override;

private:
    const ButtonMatcher& m_matcher;
};



class ButtonDetector : public VisualInferenceCallback{
public:
    ButtonDetector(
        Logger& logger, VideoOverlay& overlay,
        ButtonType type,
        const ImageFloatBox& box,
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

    ButtonTracker m_tracker;
    WhiteObjectWatcher m_watcher;

    DetectionDebouncer<bool> m_debouncer;
};



}
}
}
#endif
