/*  Selection Arrow
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_SelectionArrow_H
#define PokemonAutomation_PokemonBDSP_SelectionArrow_H

#include <deque>
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"

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

    void detect(const QImage& screen);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const QImage& frame, WallClock timestamp) override;

protected:
    VideoOverlay& m_overlay;
    Color m_color;
    ImageFloatBox m_box;
    std::deque<InferenceBoxScope> m_arrow_boxes;

private:
    size_t m_trigger_count = 0;
};



}
}
}
#endif
