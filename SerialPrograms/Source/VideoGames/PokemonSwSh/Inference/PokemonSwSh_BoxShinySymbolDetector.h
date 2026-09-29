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

#include "Common/Cpp/TestRunners/UnitTest.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"

namespace PokemonAutomation{

class ImageViewRGB32;
class VideoOverlaySet;

namespace NintendoSwitch{
namespace PokemonSwSh{


class BoxShinySymbolDetector{
public:
    BoxShinySymbolDetector()
        : m_box(0.969, 0.145, 0.024, 0.040)
    {}

    void make_overlays(VideoOverlaySet& items);
    bool detect(const ImageViewRGB32& screen);

private:
    ImageFloatBox m_box;
};




void add_tests_BoxShinySymbolDetector(UnitTestDatabase& database);



}
}
}

#endif

