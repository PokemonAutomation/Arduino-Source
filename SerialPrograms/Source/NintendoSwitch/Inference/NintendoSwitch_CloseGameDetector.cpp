/*  Close Game Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "NintendoSwitch_CloseGameDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



CloseGameDetector::CloseGameDetector(ConsoleHandle& console, Color color)
    : m_color(color)
    , m_top_box(0.226358, 0.272648, 0.407445, 0.033989)
    , m_left_box(0.225, 0.275, 0.015, 0.350)
    , m_close_game_text_row(0.330986, 0.649052, 0.342052, 0.051878)
{}
void CloseGameDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_top_box);
    items.add(m_color, m_left_box);
    items.add(m_color, m_close_game_text_row);
}


bool CloseGameDetector::detect(const ImageViewRGB32& screen){
    
    ImageStats stats_top = image_stats(extract_box_reference(screen, m_top_box));  // the top portion of the Close game popup
    ImageStats stats_left = image_stats(extract_box_reference(screen, m_left_box));  // the left portion of the Close game popup
    ImageStats stats_close_game_text = image_stats(extract_box_reference(screen, m_close_game_text_row));  // the bottom portion of the Close game popup. overlapping with the close game text.
    
//    cout << "stats_close_game_text.stddev.sum()" << stats_close_game_text.stddev.sum() << endl;
    bool white;
//    cout << "stats_top.average.sum() = " << stats_top.average.sum() << endl;
    if (stats_top.average.sum() < 300){
        white = false;
    }else if (stats_top.average.sum() > 500){
        white = true;
    }else{
        return false;
    }

//    cout << "stats_top.stddev.sum() = " << stats_top.stddev.sum() << endl;

    // ensure top is uniform in color
    if (stats_top.stddev.sum() > 20){
        return false;
    }

//    cout << "stats_left.stddev.sum() = " << stats_left.stddev.sum() << endl;
    // ensure left is uniform in color
    if (stats_left.stddev.sum() > 20){
        return false;
    }

    // ensure bottom is NOT uniform in color
    if (stats_close_game_text.stddev.sum() < 50){
        return false;
    }

    if (white){ // if top is white, ensure left is also white
        if (!is_white(stats_top) || !is_white(stats_left)){
//            cout << "top = " << stats_top.stddev << " / " << stats_top.average << endl;
//            cout << "left = " << stats_left.stddev << " / " << stats_left.average << endl;
//            cout << "asdf" << endl;
            return false;
        }
    }else{
        if (!is_grey(stats_top, 0, 300) || !is_grey(stats_left, 0, 300)){
//            cout << "qwer" << endl;
            return false;
        }
    }

    // ensure top and left are the same color.
    if (euclidean_distance(stats_top.average, stats_left.average) > 20){
//        cout << "qwer = " << euclidean_distance(stats_top.average, stats_left.average) << endl;
        return false;
    }


    return true;
}






}
}
