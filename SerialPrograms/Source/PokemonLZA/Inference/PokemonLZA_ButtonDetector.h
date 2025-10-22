/*  Button Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_ButtonDetector_H
#define PokemonAutomation_PokemonLZA_ButtonDetector_H

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
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
};


class ButtonMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    ButtonMatcher(ButtonType type, size_t min_width, size_t min_height, double max_rmsd);
    static const ButtonMatcher& A(){
        static ButtonMatcher matcher(ButtonType::ButtonA, 60, 60, 70);
        return matcher;
    }
    static const ButtonMatcher& B(){
        static ButtonMatcher matcher(ButtonType::ButtonB, 60, 60, 70);
        return matcher;
    }
    static const ButtonMatcher& X(){
        static ButtonMatcher matcher(ButtonType::ButtonX, 60, 60, 70);
        return matcher;
    }
    static const ButtonMatcher& Y(){
        static ButtonMatcher matcher(ButtonType::ButtonY, 60, 60, 70);
        return matcher;
    }
    static const ButtonMatcher& L(){
        static ButtonMatcher matcher(ButtonType::ButtonL, 60, 60, 70);
        return matcher;
    }
    static const ButtonMatcher& R(){
        static ButtonMatcher matcher(ButtonType::ButtonR, 60, 60, 70);
        return matcher;
    }
    static const ButtonMatcher& Plus(){
        static ButtonMatcher matcher(ButtonType::ButtonPlus, 60, 60, 70);
        return matcher;
    }
    static const ButtonMatcher& Minus(){
        static ButtonMatcher matcher(ButtonType::ButtonMinus, 60, 60, 70);
        return matcher;
    }
    static const ButtonMatcher& Right(){
        static ButtonMatcher matcher(ButtonType::ButtonRight, 60, 60, 70);
        return matcher;
    }

    virtual bool check_image(Resolution input_resolution, const ImageViewRGB32& image) const override{
        size_t min_width = m_min_width * input_resolution.width / 3840;
        size_t min_height = m_min_height * input_resolution.height / 2160;
        return image.width() >= min_width && image.height() >= min_height;
    };

    size_t m_min_width;
    size_t m_min_height;
    double m_max_rmsd;
};


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
