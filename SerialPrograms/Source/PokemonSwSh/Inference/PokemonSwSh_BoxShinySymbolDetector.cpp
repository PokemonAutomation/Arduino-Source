/*  Box Shiny Symbol Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonSwSh_BoxShinySymbolDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

namespace{
    ImageFloatBox SHINY_BOX{0.969, 0.145, 0.024, 0.040};
}

void BoxShinySymbolDetector::make_overlays(VideoOverlaySet& items){
    items.add(COLOR_RED, SHINY_BOX);
}

bool BoxShinySymbolDetector::detect(const ImageViewRGB32& screen){
    const ImageStats symbol = image_stats(extract_box_reference(screen, SHINY_BOX));
    if (PreloadSettings::debug().COLOR_CHECK){
        cout << "Symbol region stddev " << symbol.stddev.to_string() << " (sum " << symbol.stddev.sum() << "), threshold: 50" << endl;
    }
    return symbol.stddev.sum() > 50;
}

}
}
}
