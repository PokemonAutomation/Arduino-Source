/*  Selection Arrow
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_SelectionArrow_H
#define PokemonAutomation_PokemonBDSP_SelectionArrow_H

#include <deque>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class SelectionArrowFinder : public VisualInferenceCallback{
public:
    SelectionArrowFinder(
        VideoOverlay& overlay,
        const ImageFloatBox& box,
        Color color
    );

    bool detect(const ImageViewRGB32& screen);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

protected:
    VideoOverlay& m_overlay;
    Color m_color;
    ImageFloatBox m_box;
    std::deque<OverlayBoxScope> m_arrow_boxes;

private:
    size_t m_trigger_count = 0;
};



}
}
}
#endif
