/*  Box Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BoxShinyDetector_H
#define PokemonAutomation_PokemonBDSP_BoxShinyDetector_H

#include "CommonFramework/Inference/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class BoxShinyDetector : public StaticScreenDetector{
public:
    BoxShinyDetector(Color color = COLOR_RED);

    bool is_panel(const QImage& screen) const;

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const QImage& screen) const override;

private:
    Color m_color;
    ImageFloatBox m_symbol;
    ImageFloatBox m_background;
};




}
}
}
#endif
