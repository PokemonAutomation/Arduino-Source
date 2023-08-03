/*  Pokedex Shiny Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_PokedexShinyDetector_H
#define PokemonAutomation_PokemonSV_PokedexShinyDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Inference/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

// Detect the shiny symbol on a pokemon in the pokedex page
class PokedexShinyDetector : public StaticScreenDetector{
public:
    PokedexShinyDetector(Color color = COLOR_YELLOW);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    ImageFloatBox m_box;
};

class PokedexShinyWatcher : public DetectorToFinder<PokedexShinyDetector>{
public:
    PokedexShinyWatcher(FinderType finder_type = FinderType::PRESENT, Color color = COLOR_YELLOW)
         : DetectorToFinder("PokedexShinyWatcher", finder_type, std::chrono::milliseconds(100), color)
    {}
};


}
}
}
#endif
