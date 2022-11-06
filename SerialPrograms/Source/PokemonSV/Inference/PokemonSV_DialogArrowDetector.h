/*  Dialog Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_DialogArrowDetector_H
#define PokemonAutomation_PokemonSV_DialogArrowDetector_H

#include <vector>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class DialogArrowDetector : public StaticScreenDetector{
public:
    DialogArrowDetector(const ImageFloatBox& box, Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    std::vector<ImageFloatBox> detect_all(const ImageViewRGB32& screen) const;

protected:
    Color m_color;
    ImageFloatBox m_box;
};



class DialogArrowFinder : public VisualInferenceCallback{
public:
    ~DialogArrowFinder();
    DialogArrowFinder(VideoOverlay& overlay, const ImageFloatBox& box, Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;


protected:
    VideoOverlay& m_overlay;
    DialogArrowDetector m_detector;
    FixedLimitVector<OverlayBoxScope> m_arrows;
};




}
}
}
#endif
