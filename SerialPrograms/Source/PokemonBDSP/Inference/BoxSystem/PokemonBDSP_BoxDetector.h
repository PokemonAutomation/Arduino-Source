/*  Box Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BoxDetector_H
#define PokemonAutomation_PokemonBDSP_BoxDetector_H

#include "CommonFramework/Inference/VisualDetector.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class BoxDetector : public StaticScreenDetector{
public:
    BoxDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const QImage& screen) const override;

private:
    Color m_color;
    ImageFloatBox m_left;
    ImageFloatBox m_right;
    ImageFloatBox m_bottom;
    ImageFloatBox m_row;
};


class BoxWatcher : public BoxDetector, public VisualInferenceCallback{
public:
    using BoxDetector::BoxDetector;

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;
};



}
}
}
#endif
