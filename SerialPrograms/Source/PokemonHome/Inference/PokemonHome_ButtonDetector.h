/*  Button Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_ButtonDetector_H
#define PokemonAutomation_PokemonHome_ButtonDetector_H

#include <optional>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonHome{


enum class ButtonType{
    ButtonB,
    ButtonPlus,
};

class ButtonMatcher;




class ButtonDetector : public StaticScreenDetector{
public:
    ButtonDetector(
        Color color,
        ButtonType button_type,
        const ImageFloatBox& box,
        VideoOverlay* overlay = nullptr
    );
    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    virtual void reset_state() override { m_last_detected_box.reset(); }

private:
    Color m_color;
    const ButtonMatcher& m_matcher;
    ImageFloatBox m_box;
    VideoOverlay* m_overlay;

    ImageFloatBox m_last_detected;
    std::optional<OverlayBoxScope> m_last_detected_box;
};
class ButtonWatcher : public DetectorToFinder<ButtonDetector>{
public:
    ButtonWatcher(
        Color color,
        ButtonType button_type,
        const ImageFloatBox& box,
        VideoOverlay* overlay = nullptr,
        Milliseconds hold_duration = Milliseconds(50)
    )
         : DetectorToFinder("ButtonWatcher", hold_duration, color, button_type, box, overlay)
    {}
};


class BoxViewDetector : public StaticScreenDetector{
public:
    BoxViewDetector(VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    virtual void reset_state() override { m_button_plus_detector.reset_state(); }

private:
    ButtonDetector m_button_plus_detector;

};
class BoxViewWatcher : public DetectorToFinder<BoxViewDetector>{
public:
    BoxViewWatcher(VideoOverlay* overlay = nullptr)
        : DetectorToFinder("BoxViewWatcher", Milliseconds(100), overlay)
    {}
};


class SummaryScreenDetector : public StaticScreenDetector{
public:
    SummaryScreenDetector(VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    virtual void reset_state() override { m_button_B_detector.reset_state(); }

private:
    ButtonDetector m_button_B_detector;

};
class SummaryScreenWatcher : public DetectorToFinder<SummaryScreenDetector>{
public:
    SummaryScreenWatcher(VideoOverlay* overlay = nullptr)
        : DetectorToFinder("SummaryScreenWatcher", Milliseconds(100), overlay)
    {}
};



}
}
}
#endif
