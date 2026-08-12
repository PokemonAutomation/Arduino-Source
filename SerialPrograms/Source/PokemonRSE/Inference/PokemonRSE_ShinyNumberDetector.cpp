/*  Shiny Number Detector
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
#include "PokemonRSE_ShinyNumberDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

ShinyNumberDetector::ShinyNumberDetector(Color color)
    : m_box_number(0.025599, 0.103004, 0.125004, 0.093377)
{}
void ShinyNumberDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_box_number));
}
bool ShinyNumberDetector::read(Logger& logger, const ImageViewRGB32& frame){
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);

    const bool replace_color_within_range = true;

    //Filter out background (R/S, purple)
    ImageRGB32 filtered_region = filter_rgb32_range(
        extract_box_reference(game_screen, m_box_number),
        combine_rgb(120, 100, 185), combine_rgb(254, 251, 255), Color(0), replace_color_within_range
    );
    //Filter out background (E, green)
    ImageRGB32 filtered_region2 = filter_rgb32_range(
        filtered_region,
        combine_rgb(86, 162, 99), combine_rgb(236, 255, 240), Color(0), replace_color_within_range
    );
    ImageStats stats = image_stats(filtered_region2);

    
    filtered_region2.save("./filtered_only.png");
    cout << stats.average.r << endl;
    cout << stats.average.g << endl;
    cout << stats.average.b << endl;
    

    /*
    Shiny:
    R: 196.632, G: 196.771, B: 145.863
    Not shiny:
    R: 181.862, G: 180.686, B: 193.999
    */

    if (stats.average.b + 20 < stats.average.r){
        return true;
    }
    return false;
}


}
}
}

