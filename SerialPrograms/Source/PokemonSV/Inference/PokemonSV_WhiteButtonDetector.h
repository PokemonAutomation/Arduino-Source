/*  White Button Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_WhiteButtonDetector_H
#define PokemonAutomation_PokemonSV_WhiteButtonDetector_H

#include <vector>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


enum class WhiteButton{
    ButtonA,
    ButtonB,
    ButtonY,
    ButtonMinus,
};


class WhiteButtonMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    WhiteButtonMatcher(WhiteButton type, size_t min_width, size_t max_width, double max_rmsd);
    static const WhiteButtonMatcher& A();
    static const WhiteButtonMatcher& B();
    static const WhiteButtonMatcher& Y();
    static const WhiteButtonMatcher& Minus();

    virtual bool check_image(const ImageViewRGB32& image) const override{
        return image.width() >= m_min_width && image.height() >= m_min_height;
    };

    size_t m_min_width;
    size_t m_min_height;
    double m_max_rmsd;
};


class WhiteButtonDetector : public StaticScreenDetector{
public:
    WhiteButtonDetector(WhiteButton button, const ImageFloatBox& box, Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    std::vector<ImageFloatBox> detect_all(const ImageViewRGB32& screen) const;

protected:
    const WhiteButtonMatcher& m_matcher;
    Color m_color;
    ImageFloatBox m_box;
};



class WhiteButtonFinder : public VisualInferenceCallback{
public:
    ~WhiteButtonFinder();
    WhiteButtonFinder(WhiteButton button, VideoOverlay& overlay, const ImageFloatBox& box, Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;


protected:
    VideoOverlay& m_overlay;
    WhiteButtonDetector m_detector;
    FixedLimitVector<InferenceBoxScope> m_arrows;
};




}
}
}
#endif
