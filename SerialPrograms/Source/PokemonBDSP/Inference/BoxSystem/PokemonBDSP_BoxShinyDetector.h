/*  Box Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BoxShinyDetector_H
#define PokemonAutomation_PokemonBDSP_BoxShinyDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class BoxShinyDetector : public StaticScreenDetector{
public:
    BoxShinyDetector(Color color = COLOR_RED);

    bool is_panel(const ImageViewRGB32& screen) const;

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_symbol;
    ImageFloatBox m_background;
};




}
}
}
#endif
