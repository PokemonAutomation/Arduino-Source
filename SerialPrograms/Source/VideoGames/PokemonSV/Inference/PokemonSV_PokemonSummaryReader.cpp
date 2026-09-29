/*  Pokemon Summary Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonSV_PokemonSummaryReader.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



bool is_summary_color(const ImageStats& stats){
    return is_solid(stats, {0.648549, 0.2861580, 0.0652928}, 0.15, 25)  //  Scarlet
        || is_solid(stats, {0.367816, 0.0746615, 0.5575230}, 0.15, 25)  //  Violet
        || is_solid(stats, {0.196536, 0.5933000, 0.2101630}, 0.18, 25)  //  DLC1 Green
        || is_solid(stats, {0.169492, 0.330508 , 0.5      }, 0.18, 25)  //  DLC2 Dark Blue
        || (stats.average.g / stats.average.sum()) > 0.5;
}



PokemonSummaryDetector::PokemonSummaryDetector(Color color)
    : m_color(color)
    , m_top_blue_left(0.30, 0.09, 0.10, 0.05)
    , m_top_blue_right(0.60, 0.09, 0.35, 0.05)
    , m_bottom(0.03, 0.94, 0.40, 0.04)
    , m_arrow_left(color, WhiteButton::ButtonLeft, {0.415, 0.085, 0.035, 0.057})
    , m_arrow_right(color, WhiteButton::ButtonRight, {0.553, 0.085, 0.035, 0.057})
    , m_shiny_symbol(0.575, 0.865, 0.017, 0.030)
{}
void PokemonSummaryDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_top_blue_left);
    items.add(m_color, m_top_blue_right);
    items.add(m_color, m_bottom);
    m_arrow_left.make_overlays(items);
    m_arrow_right.make_overlays(items);
    items.add(m_color, m_shiny_symbol);
}
bool PokemonSummaryDetector::detect(const ImageViewRGB32& screen){
    ImageStats top_blue_left = image_stats(extract_box_reference(screen, m_top_blue_left));
//    cout << top_blue_left.average << top_blue_left.stddev << endl;
    if (!is_solid(top_blue_left, {0.0745162, 0.311321, 0.614163}, 0.30, 10)){
//        cout << "bad: blue left" << endl;
        return false;
    }

    ImageStats top_blue_right = image_stats(extract_box_reference(screen, m_top_blue_right));
//    cout << top_blue_right.average << top_blue_right.stddev << endl;
    if (!is_solid(top_blue_right, {0.0745162, 0.311321, 0.614163}, 0.30, 10)){
//        cout << "bad: blue right" << endl;
        return false;
    }

    ImageStats bottom = image_stats(extract_box_reference(screen, m_bottom));
//    cout << bottom.average << bottom.stddev << endl;
#if 0
    if (bottom.stddev.sum() > 20){
        return false;
    }
#else
    if (!is_summary_color(bottom)){
//        cout << "bad summary color" << endl;
        return false;
    }
#endif

    if (!m_arrow_left.detect(screen)){
//        cout << "bad arrow left" << endl;
        return false;
    }
    if (!m_arrow_right.detect(screen)){
//        cout << "bad arrow right" << endl;
        return false;
    }

//    ImageStats shiny_symbol = image_stats(extract_box_reference(screen, m_shiny_symbol));
//    cout << shiny_symbol.average << shiny_symbol.stddev << endl;

    return true;
}
bool PokemonSummaryDetector::is_shiny(const ImageViewRGB32& screen) const{
    ImageStats shiny = image_stats(extract_box_reference(screen, m_shiny_symbol));
    return shiny.stddev.sum() > 100;
}






}
}
}
