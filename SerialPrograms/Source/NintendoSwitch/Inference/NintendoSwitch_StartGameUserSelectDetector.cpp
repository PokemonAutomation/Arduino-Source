/*  Start Game User-Select Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "NintendoSwitch_StartGameUserSelectDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{




StartGameUserSelectDetector::StartGameUserSelectDetector(ConsoleHandle& console, Color color)
    : m_console(console)
    , m_type_detector(console, color)
    , m_switch1(color)
    , m_switch2(color)
{}
void StartGameUserSelectDetector::make_overlays(VideoOverlaySet& items) const{
    m_type_detector.make_overlays(items);
    m_switch1.make_overlays(items);
    m_switch2.make_overlays(items);
}
bool StartGameUserSelectDetector::detect(const ImageViewRGB32& screen){
    return detect_only(screen);
#if 0
    if (detect_only(screen)){
        m_console.state().set_console_type(m_console, m_console_type);
        return true;
    }else{
        return false;
    }
#endif
}
void StartGameUserSelectDetector::commit_state(){
    m_console.state().set_console_type(m_console, m_console_type);
}
bool StartGameUserSelectDetector::detect_only(const ImageViewRGB32& screen){
    ConsoleType type = m_type_detector.detect_only(screen);
    m_console_type = type;

    if (type == ConsoleType::Unknown){
        return false;
    }
    if (is_switch1(type)){
        return m_switch1.detect(screen);
    }
    if (is_switch2(type)){
        return m_switch2.detect(screen);
    }

    throw InternalProgramError(
        nullptr, PA_CURRENT_FUNCTION,
        "Invalid ConsoleType: " + std::to_string((int)type)
    );
}








StartGameUserSelectDetector_Switch1::StartGameUserSelectDetector_Switch1(Color color)
    : m_color(color)
    , m_bottom_row(0.10, 0.92, 0.10, 0.05)
    , m_bottom_icons(0.70, 0.92, 0.28, 0.05)
    , m_top_row(0.50, 0.47, 0.45, 0.05)
    , m_mid_row(0.10, 0.55, 0.80, 0.02)
    , m_user_slot(0.45, 0.55, 0.10, 0.35)
{}
void StartGameUserSelectDetector_Switch1::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_bottom_row);
    items.add(m_color, m_bottom_icons);
    items.add(m_color, m_top_row);
    items.add(m_color, m_mid_row);
    items.add(m_color, m_user_slot);
}
bool StartGameUserSelectDetector_Switch1::detect(const ImageViewRGB32& screen){
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


StartGameUserSelectDetector_Switch2::StartGameUserSelectDetector_Switch2(Color color)
    : m_color(color)
    , m_bottom_row(0.10, 0.92, 0.10, 0.05)
    , m_bottom_icons(0.70, 0.92, 0.28, 0.05)
    , m_top_row(0.50, 0.45, 0.45, 0.05)
    , m_mid_row(0.10, 0.53, 0.80, 0.02)
    , m_user_slot(0.45, 0.55, 0.10, 0.30)
{}
void StartGameUserSelectDetector_Switch2::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_bottom_row);
    items.add(m_color, m_bottom_icons);
    items.add(m_color, m_top_row);
    items.add(m_color, m_mid_row);
    items.add(m_color, m_user_slot);
}
bool StartGameUserSelectDetector_Switch2::detect(const ImageViewRGB32& screen){
    ImageStats stats_bottom_row = image_stats(extract_box_reference(screen, m_bottom_row));
//    cout << stats_bottom_row.average << stats_bottom_row.stddev << endl;
    if (stats_bottom_row.stddev.sum() > 10){
        return false;
    }
    bool white;
    if (is_grey(stats_bottom_row, 0, 300)){
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
        if (!is_grey(stats_top_row, 0, 300) || !is_grey(stats_mid_row, 0, 300)){
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









}
}
