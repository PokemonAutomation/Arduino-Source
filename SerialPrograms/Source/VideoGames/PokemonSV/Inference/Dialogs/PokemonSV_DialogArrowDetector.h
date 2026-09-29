/*  Dialog Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_DialogArrowDetector_H
#define PokemonAutomation_PokemonSV_DialogArrowDetector_H

#include <vector>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class DialogArrowDetector : public StaticScreenDetector{
public:
    DialogArrowDetector(Color color, const ImageFloatBox& box);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    std::vector<ImageFloatBox> detect_all(const ImageViewRGB32& screen) const;

    std::pair<double, double> locate_dialog_arrow(const ImageViewRGB32& screen) const;

protected:
    Color m_color;
    ImageFloatBox m_box;
};



class DialogArrowWatcher : public VisualInferenceCallback{
public:
    ~DialogArrowWatcher();
    DialogArrowWatcher(Color color, VideoOverlay& overlay, const ImageFloatBox& box, const double top_line, const double bottom_line);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;


protected:
    VideoOverlay& m_overlay;
    DialogArrowDetector m_detector;
    double m_top_line;
    double m_bottom_line;
    uint16_t m_num_oscillation_above_top_line;
    uint16_t m_num_oscillation_below_bottom_line;
    uint16_t m_num_no_detection;
    // FixedLimitVector<OverlayBoxScope> m_arrows;

};




}
}
}
#endif
