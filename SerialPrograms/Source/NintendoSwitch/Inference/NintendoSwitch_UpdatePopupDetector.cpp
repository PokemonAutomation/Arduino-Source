/*  Update Popup Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "NintendoSwitch_UpdatePopupDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



UpdatePopupDetector::UpdatePopupDetector(ConsoleHandle& console, Color color)
    : m_type_detector(console, color)
    , m_switch1(color)
    , m_switch2(color)
{}
void UpdatePopupDetector::make_overlays(VideoOverlaySet& items) const{
    m_type_detector.make_overlays(items);
    m_switch1.make_overlays(items);
    m_switch2.make_overlays(items);
}
bool UpdatePopupDetector::detect(const ImageViewRGB32& screen){
    return detect_only(screen);

#if 0
    if (detect_only(screen)){
        m_type_detector.commit_to_cache();
        return true;
    }else{
        return false;
    }
#endif
}
void UpdatePopupDetector::commit_state(){
    m_type_detector.commit_to_cache();
}
bool UpdatePopupDetector::detect_only(const ImageViewRGB32& screen){
    ConsoleType type = m_type_detector.detect_only(screen);

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








UpdatePopupDetector_Switch1::UpdatePopupDetector_Switch1(Color color)
    : m_color(color)
    , m_box_top(0.25, 0.26, 0.50, 0.02)
    , m_box_mid(0.25, 0.52, 0.50, 0.02)
    , m_top(0.10, 0.15, 0.80, 0.03)
    , m_left(0.08, 0.25, 0.10, 0.38)
    , m_bottom_solid(0.10, 0.84, 0.80, 0.04)
    , m_bottom_buttons(0.70, 0.92, 0.28, 0.05)
{}
void UpdatePopupDetector_Switch1::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box_top);
    items.add(m_color, m_box_mid);
    items.add(m_color, m_top);
    items.add(m_color, m_left);
    items.add(m_color, m_bottom_solid);
    items.add(m_color, m_bottom_buttons);
}
bool UpdatePopupDetector_Switch1::detect(const ImageViewRGB32& screen){
    ImageStats stats_box_top = image_stats(extract_box_reference(screen, m_box_top));
//    cout << stats_box_top.average << stats_box_top.stddev << endl;
    bool white;
    if (stats_box_top.average.sum() < 300){
        white = false;
    }else if (stats_box_top.average.sum() > 500){
        white = true;
    }else{
        return false;
    }
    if (stats_box_top.stddev.sum() > 10){
        return false;
    }

//    cout << "white: " << white << endl;

    ImageStats stats_box_mid = image_stats(extract_box_reference(screen, m_box_mid));
    if (stats_box_mid.stddev.sum() > 10){
        return false;
    }
    if (euclidean_distance(stats_box_top.average, stats_box_mid.average) > 10){
        return false;
    }

    ImageStats stats_left = image_stats(extract_box_reference(screen, m_left));
//    cout << stats_left.stddev << endl;
    if (stats_left.stddev.sum() < 30){
//        cout << "zxcv" << endl;
        return false;
    }

    ImageStats stats_top = image_stats(extract_box_reference(screen, m_top));
//    cout << stats_top.average << stats_top.stddev << endl;

    ImageStats bottom_solid = image_stats(extract_box_reference(screen, m_bottom_solid));
//    cout << bottom_solid.average << bottom_solid.stddev << endl;

    if (euclidean_distance(stats_top.average, bottom_solid.average) > 10){
//        cout << "qwer" << endl;
        return false;
    }

    if (white){
        if (!is_grey(stats_top, 100, 300) || !is_grey(bottom_solid, 100, 300)){
//            cout << "asdf" << endl;
            return false;
        }
    }else{
        if (!is_grey(stats_top, 0, 100) || !is_grey(bottom_solid, 0, 100)){
//            cout << "zxcv" << endl;
            return false;
        }
    }

    ImageStats stats_bottom_buttons = image_stats(extract_box_reference(screen, m_bottom_buttons));
//    cout << stats_bottom_buttons.average << stats_bottom_buttons.stddev << endl;
    if (stats_bottom_buttons.stddev.sum() < 30){
        return false;
    }

    return true;
}




UpdatePopupDetector_Switch2::UpdatePopupDetector_Switch2(Color color)
    : m_color(color)
    , m_box_top(0.25, 0.31, 0.50, 0.02)
//    , m_box_mid(0.25, 0.482, 0.50, 0.02)
    , m_top(0.10, 0.17, 0.80, 0.03)
    , m_left(0.08, 0.25, 0.10, 0.38)
    , m_bottom_solid(0.10, 0.86, 0.80, 0.04)
    , m_bottom_buttons(0.70, 0.92, 0.28, 0.05)
{}

void UpdatePopupDetector_Switch2::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box_top);
//    items.add(m_color, m_box_mid);
    items.add(m_color, m_top);
    items.add(m_color, m_left);
    items.add(m_color, m_bottom_solid);
    items.add(m_color, m_bottom_buttons);
}
bool UpdatePopupDetector_Switch2::detect(const ImageViewRGB32& screen){
    ImageStats stats_box_top = image_stats(extract_box_reference(screen, m_box_top));
//    cout << stats_box_top.average << stats_box_top.stddev << endl;
    bool white;
    if (stats_box_top.average.sum() < 300){
        white = false;
    }else if (stats_box_top.average.sum() > 500){
        white = true;
    }else{
        return false;
    }
    if (stats_box_top.stddev.sum() > 10){
        return false;
    }

//    cout << "white: " << white << endl;

#if 0
    ImageStats stats_box_mid = image_stats(extract_box_reference(screen, m_box_mid));
    if (stats_box_mid.stddev.sum() > 10){
        return false;
    }
    if (euclidean_distance(stats_box_top.average, stats_box_mid.average) > 10){
        return false;
    }
#endif

    ImageStats stats_left = image_stats(extract_box_reference(screen, m_left));
//    cout << stats_left.stddev << endl;
    if (stats_left.stddev.sum() < 30){
//        cout << "zxcv" << endl;
        return false;
    }

    ImageStats stats_top = image_stats(extract_box_reference(screen, m_top));
//    cout << stats_top.average << stats_top.stddev << endl;

    ImageStats bottom_solid = image_stats(extract_box_reference(screen, m_bottom_solid));
//    cout << bottom_solid.average << bottom_solid.stddev << endl;

    if (euclidean_distance(stats_top.average, bottom_solid.average) > 10){
//        cout << "qwer" << endl;
        return false;
    }

    do{
        if (white){
            if (is_grey(stats_top, 200, 765) && is_grey(bottom_solid, 200, 765)){
                break;
            }
        }else{
            if (is_grey(stats_top, 0, 100) && is_grey(bottom_solid, 0, 100)){
                break;
            }
            if (is_grey(stats_top, 200, 500) && is_grey(bottom_solid, 200, 500)){
                break;
            }
        }

        return false;
    }while (false);

    ImageStats stats_bottom_buttons = image_stats(extract_box_reference(screen, m_bottom_buttons));
//    cout << stats_bottom_buttons.average << stats_bottom_buttons.stddev << endl;
    if (stats_bottom_buttons.stddev.sum() < 30){
        return false;
    }

    return true;
}









}
}
