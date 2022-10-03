/*  Detect Home
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "NintendoSwitch_DetectHome.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

HomeDetector::HomeDetector()
    : m_bottom_row(0.15, 0.92, 0.30, 0.05)
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
    if (stats_bottom_row.average.sum() < 200){
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

    ImageStats stats_bottom_left = image_stats(extract_box_reference(screen, m_bottom_left));
    ImageStats stats_bottom_right = image_stats(extract_box_reference(screen, m_bottom_right));
//    cout << stats_bottom_left.average << stats_bottom_left.stddev << endl;
//    cout << stats_bottom_right.average << stats_bottom_right.stddev << endl;
    if (white){
        if (!is_white(stats_bottom_left) || !is_white(stats_bottom_right)){
            return false;
        }
    }else{
        if (!is_grey(stats_bottom_left, 0, 150) || !is_grey(stats_bottom_right, 0, 150)){
            return false;
        }
    }

    if (euclidean_distance(stats_bottom_row.average, stats_bottom_left.average) > 20){
        return false;
    }
    if (euclidean_distance(stats_bottom_row.average, stats_bottom_right.average) > 20){
        return false;
    }
    if (euclidean_distance(stats_bottom_left.average, stats_bottom_right.average) > 20){
        return false;
    }

    ImageStats stats_user_icons = image_stats(extract_box_reference(screen, m_user_icons));
//    cout << stats_user_icons.stddev << endl;
    if (stats_user_icons.stddev.sum() < 50){
        return false;
    }
    ImageStats stats_game_slot = image_stats(extract_box_reference(screen, m_game_slot));
//    cout << stats_game_slot.stddev << endl;
    if (stats_game_slot.stddev.sum() < 50){
        return false;
    }

    return true;
}
HomeWatcher::HomeWatcher()
    : VisualInferenceCallback("HomeWatcher")
{}
void HomeWatcher::make_overlays(VideoOverlaySet& items) const{
    HomeDetector::make_overlays(items);
}
bool HomeWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return HomeDetector::detect(frame);
}







StartGameUserSelectDetector::StartGameUserSelectDetector()
    : m_bottom_row(0.15, 0.92, 0.30, 0.05)
    , m_bottom_icons(0.70, 0.92, 0.28, 0.05)
    , m_top_row(0.50, 0.47, 0.45, 0.05)
    , m_user_slot(0.45, 0.55, 0.10, 0.35)
{}
void StartGameUserSelectDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_bottom_row);
    items.add(COLOR_RED, m_bottom_icons);
    items.add(COLOR_RED, m_top_row);
    items.add(COLOR_RED, m_user_slot);
}
bool StartGameUserSelectDetector::detect(const ImageViewRGB32& screen) const{
    ImageStats stats_bottom_row = image_stats(extract_box_reference(screen, m_bottom_row));
//    cout << stats_bottom_row.average << stats_bottom_row.stddev << endl;
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
    if (white){
        if (!is_white(stats_top_row)){
            return false;
        }
    }else{
        if (!is_grey(stats_top_row, 50, 300)){
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
StartGameUserSelectWatcher::StartGameUserSelectWatcher()
    : VisualInferenceCallback("StartGameUserSelectWatcher")
{}
void StartGameUserSelectWatcher::make_overlays(VideoOverlaySet& items) const{
    StartGameUserSelectDetector::make_overlays(items);
}
bool StartGameUserSelectWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return StartGameUserSelectDetector::detect(frame);
}





































}
}
