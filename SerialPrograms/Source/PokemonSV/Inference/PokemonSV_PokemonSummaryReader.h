/*  Pokemon Summary Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_PokemonSummaryReader_H
#define PokemonAutomation_PokemonSV_PokemonSummaryReader_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"
#include "PokemonSV_WhiteButtonDetector.h"

namespace PokemonAutomation{
    struct ImageStats;
namespace NintendoSwitch{
namespace PokemonSV{


bool is_summary_color(const ImageStats& stats);


// Detect pokemon status summary page
class PokemonSummaryDetector : public StaticScreenDetector{
public:
    PokemonSummaryDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    bool is_shiny(const ImageViewRGB32& screen) const;


protected:
    Color m_color;
    ImageFloatBox m_top_blue_left;
    ImageFloatBox m_top_blue_right;
    ImageFloatBox m_bottom;

    WhiteButtonDetector m_arrow_left;
    WhiteButtonDetector m_arrow_right;

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
