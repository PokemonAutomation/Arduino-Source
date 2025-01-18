/*  Detect Home
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "NintendoSwitch_DetectHome.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

HomeDetector::HomeDetector()
    : m_bottom_row(0.10, 0.92, 0.10, 0.05)
    , m_bottom_icons(0.70, 0.92, 0.28, 0.05)
    , m_bottom_left(0.02, 0.70, 0.15, 0.15)
    , m_bottom_right(0.83, 0.70, 0.15, 0.15)
    , m_user_icons(0.05, 0.05, 0.2, 0.08)
    , m_game_slot(0.08, 0.25, 0.10, 0.38)
{}
void HomeDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_bottom_row);
    items.add(COLOR_RED, m_bottom_icons);
    items.add(COLOR_RED, m_bottom_left);
    items.add(COLOR_RED, m_bottom_right);
    items.add(COLOR_RED, m_user_icons);
    items.add(COLOR_RED, m_game_slot);
}
bool HomeDetector::detect(const ImageViewRGB32& screen) const{
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

    return true;
}







StartGameUserSelectDetector::StartGameUserSelectDetector(Color color)
    : m_color(color)
    , m_bottom_row(0.10, 0.92, 0.10, 0.05)
    , m_bottom_icons(0.70, 0.92, 0.28, 0.05)
    , m_top_row(0.50, 0.47, 0.45, 0.05)
    , m_mid_row(0.10, 0.55, 0.80, 0.02)
    , m_user_slot(0.45, 0.55, 0.10, 0.35)
{}
void StartGameUserSelectDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_bottom_row);
    items.add(m_color, m_bottom_icons);
    items.add(m_color, m_top_row);
    items.add(m_color, m_mid_row);
    items.add(m_color, m_user_slot);
}
bool StartGameUserSelectDetector::detect(const ImageViewRGB32& screen) const{
    ImageStats stats_bottom_row = image_stats(extract_box_reference(screen, m_bottom_row));
//    cout << stats_bottom_row.average << stats_bottom_row.stddev << endl;
    if (stats_bottom_row.stddev.sum() > 10){
        return false;
    }
    bool white;
    if (is_grey(stats_bottom_row, 50, 300)){
        white = false;
    }else if (stats_bottom_row.average.sum() > 500){
        white = true;
    }else{
        return false;
    }

    ImageStats stats_bottom_icons = image_stats(extract_box_reference(screen, m_bottom_icons));
//    cout << stats_bottom_icons.average << stats_bottom_icons.stddev << endl;
    if (stats_bottom_icons.stddev.sum() < 50){
        return false;
    }

    ImageStats stats_top_row = image_stats(extract_box_reference(screen, m_top_row));
    ImageStats stats_mid_row = image_stats(extract_box_reference(screen, m_mid_row));
//    cout << stats_top_row.average << stats_top_row.stddev << endl;
    if (stats_top_row.stddev.sum() > 10 || stats_mid_row.stddev.sum() > 10){
        return false;
    }
    if (white){
        if (!is_white(stats_top_row) || !is_white(stats_mid_row)){
            return false;
        }
    }else{
        if (!is_grey(stats_top_row, 50, 300) || !is_grey(stats_mid_row, 50, 300)){
            return false;
        }
    }

    if (euclidean_distance(stats_bottom_row.average, stats_top_row.average) > 20){
        return false;
    }

    ImageStats stats_user_slot = image_stats(extract_box_reference(screen, m_user_slot));
//    cout << stats_user_slot.average << stats_user_slot.stddev << endl;
    if (stats_user_slot.stddev.sum() < 50){
        return false;
    }

    return true;
}






UpdateMenuDetector::UpdateMenuDetector(Color color, bool invert)
    : m_color(color)
    , m_invert(invert)
    , m_box_top(0.25, 0.26, 0.50, 0.02)
    , m_box_mid(0.25, 0.52, 0.50, 0.02)
    , m_top(0.10, 0.15, 0.80, 0.03)
    , m_left(0.08, 0.25, 0.10, 0.38)
    , m_bottom_solid(0.10, 0.84, 0.80, 0.04)
    , m_bottom_buttons(0.70, 0.92, 0.28, 0.05)
{}
void UpdateMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box_top);
    items.add(m_color, m_box_mid);
    items.add(m_color, m_top);
    items.add(m_color, m_left);
    items.add(m_color, m_bottom_solid);
    items.add(m_color, m_bottom_buttons);
}
bool UpdateMenuDetector::detect(const ImageViewRGB32& screen) const{
    ImageStats stats_box_top = image_stats(extract_box_reference(screen, m_box_top));
//    cout << stats_box_top.average << stats_box_top.stddev << endl;
    bool white;
    if (stats_box_top.average.sum() < 300){
        white = false;
    }else if (stats_box_top.average.sum() > 500){
        white = true;
    }else{
        return m_invert;
    }
    if (stats_box_top.stddev.sum() > 10){
        return m_invert;
    }

//    cout << "white: " << white << endl;

    ImageStats stats_box_mid = image_stats(extract_box_reference(screen, m_box_mid));
    if (stats_box_mid.stddev.sum() > 10){
        return m_invert;
    }
    if (euclidean_distance(stats_box_top.average, stats_box_mid.average) > 10){
        return m_invert;
    }

    ImageStats stats_left = image_stats(extract_box_reference(screen, m_left));
//    cout << stats_left.stddev << endl;
    if (stats_left.stddev.sum() < 30){
//        cout << "zxcv" << endl;
        return m_invert;
    }

    ImageStats stats_top = image_stats(extract_box_reference(screen, m_top));
//    cout << stats_top.average << stats_top.stddev << endl;

    ImageStats bottom_solid = image_stats(extract_box_reference(screen, m_bottom_solid));
//    cout << bottom_solid.average << bottom_solid.stddev << endl;

    if (euclidean_distance(stats_top.average, bottom_solid.average) > 10){
//        cout << "qwer" << endl;
        return m_invert;
    }

    if (white){
        if (!is_grey(stats_top, 100, 300) || !is_grey(bottom_solid, 100, 300)){
//            cout << "asdf" << endl;
            return m_invert;
        }
    }else{
        if (!is_grey(stats_top, 0, 100) || !is_grey(bottom_solid, 0, 100)){
//            cout << "zxcv" << endl;
            return m_invert;
        }
    }

    ImageStats stats_bottom_buttons = image_stats(extract_box_reference(screen, m_bottom_buttons));
//    cout << stats_bottom_buttons.average << stats_bottom_buttons.stddev << endl;
    if (stats_bottom_buttons.stddev.sum() < 30){
        return m_invert;
    }

    return !m_invert;
}






CheckOnlineDetector::CheckOnlineDetector(Color color, bool invert)
    : m_color(color)
    , m_invert(invert)
    , m_box_top(0.25, 0.32, 0.50, 0.02)
    , m_box_mid(0.25, 0.57, 0.50, 0.02)
    , m_top(0.10, 0.15, 0.80, 0.03)
    , m_left(0.08, 0.25, 0.10, 0.38)
    , m_bottom_solid(0.10, 0.84, 0.80, 0.04)
    , m_bottom_buttons(0.70, 0.92, 0.28, 0.05)
{}
void CheckOnlineDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box_top);
    items.add(m_color, m_box_mid);
    items.add(m_color, m_top);
    items.add(m_color, m_left);
    items.add(m_color, m_bottom_solid);
    items.add(m_color, m_bottom_buttons);
}
bool CheckOnlineDetector::detect(const ImageViewRGB32& screen) const{
    ImageStats stats_box_top = image_stats(extract_box_reference(screen, m_box_top));
//    cout << stats_box_top.average << stats_box_top.stddev << endl;
    bool white;
    if (stats_box_top.average.sum() < 300){
        white = false;
    }else if (stats_box_top.average.sum() > 500){
        white = true;
    }else{
        return m_invert;
    }
    if (stats_box_top.stddev.sum() > 10){
        return m_invert;
    }

//    cout << "white: " << white << endl;

    ImageStats stats_box_mid = image_stats(extract_box_reference(screen, m_box_mid));
    if (stats_box_mid.stddev.sum() > 10){
        return m_invert;
    }
    if (euclidean_distance(stats_box_top.average, stats_box_mid.average) > 10){
        return m_invert;
    }

    ImageStats stats_left = image_stats(extract_box_reference(screen, m_left));
//    cout << stats_left.stddev << endl;
    if (stats_left.stddev.sum() < 30){
//        cout << "zxcv" << endl;
        return m_invert;
    }

    ImageStats stats_top = image_stats(extract_box_reference(screen, m_top));
//    cout << stats_top.average << stats_top.stddev << endl;

    ImageStats bottom_solid = image_stats(extract_box_reference(screen, m_bottom_solid));
//    cout << bottom_solid.average << bottom_solid.stddev << endl;

    if (euclidean_distance(stats_top.average, bottom_solid.average) > 10){
//        cout << "qwer" << endl;
        return m_invert;
    }

    if (white){
        if (!is_grey(stats_top, 100, 300) || !is_grey(bottom_solid, 100, 300)){
//            cout << "asdf" << endl;
            return m_invert;
        }
    }else{
        if (!is_grey(stats_top, 0, 100) || !is_grey(bottom_solid, 0, 100)){
//            cout << "zxcv" << endl;
            return m_invert;
        }
    }

    ImageStats stats_bottom_buttons = image_stats(extract_box_reference(screen, m_bottom_buttons));
//    cout << stats_bottom_buttons.average << stats_bottom_buttons.stddev << endl;
    if (stats_bottom_buttons.stddev.sum() < 30){
        return m_invert;
    }

    return !m_invert;
}































}
}
