/*  Tera Rewards Menu
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraRewardsMenu_H
#define PokemonAutomation_PokemonSV_TeraRewardsMenu_H

#include "CommonTools/VisualDetector.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class TeraRewardsMenuDetector : public StaticScreenDetector{
public:
    TeraRewardsMenuDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

protected:
    Color m_color;
    WhiteButtonDetector m_next_button;
    GradientArrowDetector m_arrow;
};
class TeraRewardsMenuWatcher : public DetectorToFinder<TeraRewardsMenuDetector>{
public:
    TeraRewardsMenuWatcher(Color color, std::chrono::milliseconds duration)
         : DetectorToFinder("TeraRewardsMenuWatcher", duration, color)
    {}
};



}
}
}
#endif
