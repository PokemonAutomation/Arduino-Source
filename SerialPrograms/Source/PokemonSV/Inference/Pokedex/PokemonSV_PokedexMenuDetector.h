/*  Pokedex Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_PokedexMenuDetector_H
#define PokemonAutomation_PokemonSV_PokedexMenuDetector_H

#include <functional>
#include <atomic>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Inference/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class RecentlyBattledMenuDetector : public StaticScreenDetector{
public:
    RecentlyBattledMenuDetector(Color color, const ImageFloatBox& box = {0.23, 0.23, 0.04, 0.08});

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

protected:
    Color m_color;
    ImageFloatBox m_box;
};
class RecentlyBattledMenuWatcher : public DetectorToFinder<RecentlyBattledMenuDetector>{
public:
    RecentlyBattledMenuWatcher(Color color = COLOR_RED)
         : DetectorToFinder("RecentlyBattledMenuWatcher", std::chrono::milliseconds(100), color)
    {}
};



}
}
}
#endif
