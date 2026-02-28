/*  Shiny Symbol Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "PokemonFRLG_ShinySymbolDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

ShinySymbolDetector::ShinySymbolDetector(Color color)
    : m_box_symbol(0.436, 0.211, 0.033, 0.060)
{}
void ShinySymbolDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box_symbol);
}
bool ShinySymbolDetector::read(Logger& logger, const ImageViewRGB32& frame){
    const bool replace_color_within_range = false;

    //Filter out background
    ImageRGB32 filtered_region = filter_rgb32_range(
        extract_box_reference(frame, m_box_symbol),
        combine_rgb(193, 152, 0), combine_rgb(255, 255, 162), Color(0), replace_color_within_range
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
    R: 195.786, G: 182.143, B: 142.286
    */

    if (stats.average.b + 20 < stats.average.r){
        return true;
    }
    return false;
}


}
}
}

