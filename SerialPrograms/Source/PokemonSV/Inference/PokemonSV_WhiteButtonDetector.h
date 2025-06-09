/*  White Button Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Warning: This detector is currently very prone to false positives.
 *  It is much worse than the PLA buttons because these one have transparent
 *  text which must be stored as zero alpha.
 *
 */

#ifndef PokemonAutomation_PokemonSV_WhiteButtonDetector_H
#define PokemonAutomation_PokemonSV_WhiteButtonDetector_H

#include <vector>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


enum class WhiteButton{
    ButtonA,
    ButtonA_DarkBackground,
    ButtonB,
    ButtonX,
    ButtonY,
    ButtonMinus,
    ButtonLeft,
    ButtonRight,
    ButtonLStick,
};


class WhiteButtonMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    WhiteButtonMatcher(WhiteButton type, size_t min_width, size_t max_width, double max_rmsd);
    static const WhiteButtonMatcher& A();
    static const WhiteButtonMatcher& A_DarkBackground();
    static const WhiteButtonMatcher& B();
    static const WhiteButtonMatcher& X();
    static const WhiteButtonMatcher& Y();
    static const WhiteButtonMatcher& Minus();
    static const WhiteButtonMatcher& ArrowLeft();
    static const WhiteButtonMatcher& ArrowRight();
    static const WhiteButtonMatcher& LStick();

    virtual bool check_image(const ImageViewRGB32& image) const override{
        return image.width() >= m_min_width && image.height() >= m_min_height;
    };

    size_t m_min_width;
    size_t m_min_height;
    double m_max_rmsd;
};


class WhiteButtonDetector : public StaticScreenDetector{
public:
    WhiteButtonDetector(
        Color color,
        WhiteButton button,
        const ImageFloatBox& box
    );

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    std::vector<ImageFloatBox> detect_all(const ImageViewRGB32& screen) const;

protected:
    const WhiteButtonMatcher& m_matcher;
    Color m_color;
    ImageFloatBox m_box;
};
class WhiteButtonWatcher : public DetectorToFinder<WhiteButtonDetector>{
public:
    WhiteButtonWatcher(
        Color color,
        WhiteButton button,
        const ImageFloatBox& box,
        FinderType finder_type = FinderType::PRESENT,
        std::chrono::milliseconds duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("WhiteButtonWatcher", finder_type, duration, color, button, box)
    {}
};


#if 0
class WhiteButtonWatcher : public VisualInferenceCallback{
public:
    ~WhiteButtonWatcher();
    WhiteButtonWatcher(
        Color color,
        WhiteButton button, size_t consecutive_detections,
        VideoOverlay& overlay,
        const ImageFloatBox& box
    );

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;


protected:
    VideoOverlay& m_overlay;
    WhiteButtonDetector m_detector;
    FixedLimitVector<OverlayBoxScope> m_arrows;
    size_t m_consecutive_detections;
    size_t m_trigger_count = 0;
};
#endif



}
}
}
#endif
