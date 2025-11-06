/*  Button Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_ButtonDetector_H
#define PokemonAutomation_PokemonLZA_ButtonDetector_H

#include <optional>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonLZA{


enum class ButtonType{
    ButtonA,
    ButtonB,
    ButtonX,
    ButtonY,
    ButtonL,
    ButtonR,
    ButtonPlus,
    ButtonMinus,
    ButtonRight,
    RightStickUpDown,
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
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("ButtonWatcher", hold_duration, color, button_type, box, overlay)
    {}
};





}
}
}
#endif
