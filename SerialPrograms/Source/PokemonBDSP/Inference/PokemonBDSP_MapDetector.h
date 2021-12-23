/*  Map Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_MapDetector_H
#define PokemonAutomation_PokemonBDSP_MapDetector_H

#include "CommonFramework/Inference/VisualDetector.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class MapDetector : public StaticScreenDetector{
public:
    MapDetector(QColor color = Qt::red);

    virtual void make_overlays(OverlaySet& items) const override;
    virtual bool detect(const QImage& screen) const override;

private:
    QColor m_color;
    ImageFloatBox m_box0;
    ImageFloatBox m_box1;
    ImageFloatBox m_box2;
};


class MapWatcher : public MapDetector, public VisualInferenceCallback{
public:
    using MapDetector::MapDetector;

    virtual void make_overlays(OverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;
};




}
}
}
#endif
