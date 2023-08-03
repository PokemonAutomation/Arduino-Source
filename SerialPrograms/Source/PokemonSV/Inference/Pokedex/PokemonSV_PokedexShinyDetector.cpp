/*  Pokedex Shiny Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "PokemonSV_PokedexShinyDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


PokedexShinyDetector::PokedexShinyDetector(Color color)
: m_color(color), m_box{0.187, 0.196, 0.028, 0.046} {}

void PokedexShinyDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool PokedexShinyDetector::detect(const ImageViewRGB32& frame) const{
    const auto stats = image_stats(extract_box_reference(frame, m_box));
    return stats.stddev.sum() > 100;
}


}
}
}
