/*  Shiny Symbol Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "PokemonLGPE_ShinySymbolDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

ShinySymbolDetector::ShinySymbolDetector(Color color)
    : m_box_star(0.204, 0.095, 0.033, 0.053)
{}
void ShinySymbolDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box_star);
}
bool ShinySymbolDetector::read(Logger& logger, const ImageViewRGB32& frame){
    const bool replace_color_within_range = true;

    //Filter out background
    ImageRGB32 filtered_region = filter_rgb32_range(
        extract_box_reference(frame, m_box_star),
        combine_rgb(138, 97, 221), combine_rgb(200, 181, 239), Color(0), replace_color_within_range
    );
    ImageStats stats = image_stats(filtered_region);

    /*
    filtered_region.save("./filtered_only.png");
    cout << stats.average.r << endl;
    cout << stats.average.g << endl;
    cout << stats.average.b << endl;
    */

    /*
    Shiny:
    R: 196.632, G: 196.771, B: 145.863
    Not shiny:
    R: 181.862, G: 180.686, B: 193.999
    */

    if (stats.average.r + 100 > stats.average.b){
        return true;
    }
    return false;
}


}
}
}

