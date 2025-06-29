/*  Home Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "NintendoSwitch_HomeMenuDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{




HomeMenuDetector::HomeMenuDetector(ConsoleHandle& console, Color color)
    : m_color(color)
    , m_console_type(console, color)
    , m_top(0.510223, 0.019835, 0.441450, 0.034711)
    , m_bottom_row(0.10, 0.92, 0.10, 0.05)
    , m_bottom_icons(0.70, 0.92, 0.28, 0.05)
    , m_bottom_left(0.02, 0.70, 0.15, 0.15)
    , m_bottom_right(0.83, 0.70, 0.15, 0.15)
    , m_user_icons(0.05, 0.05, 0.2, 0.08)
    , m_game_slot(0.08, 0.25, 0.10, 0.38)
{}
void HomeMenuDetector::make_overlays(VideoOverlaySet& items) const{
    m_console_type.make_overlays(items);
    items.add(m_color, m_top);
    items.add(m_color, m_bottom_row);
    items.add(m_color, m_bottom_icons);
    items.add(m_color, m_bottom_left);
    items.add(m_color, m_bottom_right);
    items.add(m_color, m_user_icons);
    items.add(m_color, m_game_slot);
}

//
//  This miraculously works on both Switch 1 and Switch 2.
//
bool HomeMenuDetector::detect(const ImageViewRGB32& screen){
    if (detect_only(screen)){
        m_console_type.commit_to_cache();
        return true;
    }else{
        return false;
    }
}
bool HomeMenuDetector::detect_only(const ImageViewRGB32& screen){
    ImageStats stats_bottom_row = image_stats(extract_box_reference(screen, m_bottom_row));
//    cout << stats_bottom_row.average << stats_bottom_row.stddev << endl;
    bool white;
//    cout << "stats_bottom_row.average.sum() = " << stats_bottom_row.average.sum() << endl;
    if (stats_bottom_row.average.sum() < 200){
        white = false;
    }else if (stats_bottom_row.average.sum() > 500){
        white = true;
    }else{
        return false;
    }

//    cout << "white: " << white << endl;

    ImageStats stats_top = image_stats(extract_box_reference(screen, m_top));
//    cout << stats_top.average << stats_top.stddev << endl;
    if (stats_top.stddev.sum() > 20){
        return false;
    }

    ImageStats stats_bottom_icons = image_stats(extract_box_reference(screen, m_bottom_icons));
//    cout << stats_bottom_icons.average << stats_bottom_icons.stddev << endl;
    if (stats_bottom_icons.stddev.sum() < 50){
        return false;
    }

    ImageStats stats_bottom_left = image_stats(extract_box_reference(screen, m_bottom_left));
    ImageStats stats_bottom_right = image_stats(extract_box_reference(screen, m_bottom_right));
//    cout << stats_bottom_left.average << stats_bottom_left.stddev << endl;
//    cout << stats_bottom_right.average << stats_bottom_right.stddev << endl;
    if (white){
        if (!is_white(stats_bottom_left) || !is_white(stats_bottom_right)){
//            cout << "asdf" << endl;
            return false;
        }
    }else{
        if (!is_grey(stats_bottom_left, 0, 200) || !is_grey(stats_bottom_right, 0, 200)){
//            cout << "qwer" << endl;
            return false;
        }
    }

//    cout << euclidean_distance(stats_bottom_row.average, stats_bottom_left.average) << endl;
    if (euclidean_distance(stats_bottom_row.average, stats_bottom_left.average) > 20){
//        cout << "qwer = " << euclidean_distance(stats_bottom_row.average, stats_bottom_left.average) << endl;
        return false;
    }
//    cout << euclidean_distance(stats_bottom_row.average, stats_bottom_right.average) << endl;
    if (euclidean_distance(stats_bottom_row.average, stats_bottom_right.average) > 20){
//        cout << "asdf" << endl;
        return false;
    }
//    cout << euclidean_distance(stats_bottom_left.average, stats_bottom_left.average) << endl;
    if (euclidean_distance(stats_bottom_left.average, stats_bottom_right.average) > 20){
//        cout << "zxcv" << endl;
        return false;
    }
//    cout << euclidean_distance(stats_top.average, stats_bottom_left.average) << endl;
    if (euclidean_distance(stats_top.average, stats_bottom_left.average) > 20){
//        cout << "xcvb" << endl;
        return false;
    }

    ImageStats stats_user_icons = image_stats(extract_box_reference(screen, m_user_icons));
//    cout << stats_user_icons.stddev << endl;
    if (stats_user_icons.stddev.sum() < 40){
        return false;
    }
    ImageStats stats_game_slot = image_stats(extract_box_reference(screen, m_game_slot));
//    cout << stats_game_slot.stddev << endl;
    if (stats_game_slot.stddev.sum() < 50){
        return false;
    }

    //  Read the console type only when we have confirmed the home menu.
    m_console_type.detect_only(screen);

    return true;
}






}
}
