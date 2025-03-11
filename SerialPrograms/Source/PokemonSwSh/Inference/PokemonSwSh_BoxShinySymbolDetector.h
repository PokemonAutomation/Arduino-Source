/*  Box Shiny Symbol Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 *  Check shiny symbol when viewing pokemon in a storage box.
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BoxShinySymbolDetector_H
#define PokemonAutomation_PokemonSwSh_BoxShinySymbolDetector_H


namespace PokemonAutomation{

class ImageViewRGB32;
class VideoOverlaySet;

namespace NintendoSwitch{
namespace PokemonSwSh{


class BoxShinySymbolDetector{
public:
    static void make_overlays(VideoOverlaySet& items);

    static bool detect(const ImageViewRGB32& screen);
};



}
}
}

#endif

