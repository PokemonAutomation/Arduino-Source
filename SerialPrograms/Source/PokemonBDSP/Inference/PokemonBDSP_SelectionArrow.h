/*  Selection Arrow
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_SelectionArrow_H
#define PokemonAutomation_PokemonBDSP_SelectionArrow_H

#include <deque>
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class SelectionArrowFinder : public VisualInferenceCallback{
public:
    SelectionArrowFinder(
        VideoOverlay& overlay,
        const ImageFloatBox& box,
        QColor color
    );

    void detect(const QImage& screen);

    virtual void make_overlays(OverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;

protected:
    VideoOverlay& m_overlay;
    QColor m_color;
    ImageFloatBox m_box;
    std::deque<InferenceBoxScope> m_arrow_boxes;
};



}
}
}
#endif
