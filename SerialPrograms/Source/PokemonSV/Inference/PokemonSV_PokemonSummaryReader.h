/*  Pokemon Summary Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_PokemonSummaryReader_H
#define PokemonAutomation_PokemonSV_PokemonSummaryReader_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Inference/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class PokemonSummaryDetector : public StaticScreenDetector{
public:
    PokemonSummaryDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    bool is_shiny(const ImageViewRGB32& screen) const;


protected:
    Color m_color;
    ImageFloatBox m_top_blue_left;
    ImageFloatBox m_top_blue_right;
    ImageFloatBox m_bottom;

    ImageFloatBox m_shiny_symbol;
};
class PokemonSummaryWatcher : public DetectorToFinder<PokemonSummaryDetector>{
public:
    PokemonSummaryWatcher(Color color = COLOR_RED)
         : DetectorToFinder("PokemonSummaryWatcher", std::chrono::milliseconds(250), color)
    {}
};



}
}
}
#endif
