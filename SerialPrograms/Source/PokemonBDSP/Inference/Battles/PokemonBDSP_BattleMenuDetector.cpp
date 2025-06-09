/*  Battle Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonBDSP_BattleMenuDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


BattleMenuDetector::BattleMenuDetector(BattleType battle_type, Color color)
    : m_color(color)
    , m_battle_type(battle_type)
    , m_left0_status    (0.140, 0.922, 0.100, 0.010)
    , m_left1_status    (0.140, 0.910, 0.100, 0.010)
    , m_right_status    (0.405, 0.925, 0.100, 0.010)
//    , m_opponent_left   (0.685, 0.065, 0.020, 0.030)
//    , m_opponent_right  (0.960, 0.065, 0.020, 0.030)
    , m_opponent_left   (0.708, 0.070, 0.005, 0.028)
    , m_opponent_right  (0.982, 0.070, 0.005, 0.028)
    , m_ball_left       (0.890, 0.475, 0.02, 0.03)
    , m_ball_right      (0.960, 0.475, 0.03, 0.03)
    , m_menu_battle     (0.817, 0.585 + 0 * 0.1075, 0.150, 0.070)
    , m_menu_pokemon    (0.817, 0.585 + 1 * 0.1075, 0.150, 0.070)
    , m_menu_bag        (0.817, 0.585 + 2 * 0.1075, 0.150, 0.070)
    , m_menu_run        (0.817, 0.585 + 3 * 0.1075, 0.150, 0.070)
{}
void BattleMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_left0_status);
    items.add(m_color, m_left1_status);
    items.add(m_color, m_right_status);
    items.add(m_color, m_opponent_left);
    items.add(m_color, m_opponent_right);
    items.add(m_color, m_ball_left);
    items.add(m_color, m_ball_right);
    items.add(m_color, m_menu_battle);
    items.add(m_color, m_menu_pokemon);
    items.add(m_color, m_menu_bag);
    items.add(m_color, m_menu_run);
}
bool BattleMenuDetector::is_battle_button(const ImageViewRGB32& screen) const{
    ImageStats stats = image_stats(extract_box_reference(screen, m_menu_battle));
//    cout << stats.average << stats.stddev << endl;
    double stddev = stats.stddev.sum();
    if (stddev < 30 || stddev > 150){
        return false;
    }
    double average = stats.average.sum();
    FloatPixel actual = stats.average / average;
    if (euclidean_distance(actual, {0.541961, 0.231317, 0.226722}) > 0.2){
        return false;
    }
    return true;
}
bool BattleMenuDetector::detect(const ImageViewRGB32& screen){
    {
        bool left = is_white(extract_box_reference(screen, m_opponent_left));
        bool right = is_white(extract_box_reference(screen, m_opponent_right));
        if (!left && !right){
//            cout << "opp left or right" << endl;
            return false;
        }
    }
    {
        bool left0 = is_white(extract_box_reference(screen, m_left0_status));
        bool left1 = is_white(extract_box_reference(screen, m_left1_status));
        bool right = is_white(extract_box_reference(screen, m_right_status));
        if (!left0 && !left1 && !right){
            return false;
        }
    }

    if (m_battle_type == BattleType::STARTER){
        return is_battle_button(screen);
    }


    if (m_battle_type == BattleType::STANDARD){
        if (!is_white(extract_box_reference(screen, m_ball_left))){
//            cout << "Not white" << endl;
            return false;
        }
        if (!is_white(extract_box_reference(screen, m_ball_right))){
//            cout << "Not white" << endl;
            return false;
        }
    }

    if (!is_battle_button(screen)){
        return false;
    }
//    cout << "Start Checking Pokemon" << endl;
    {
        ImageStats stats = image_stats(extract_box_reference(screen, m_menu_pokemon));
//        cout << stats.average << stats.stddev << endl;
        double stddev = stats.stddev.sum();
        if (stddev < 30 || stddev > 150){
            return false;
        }
        double average = stats.average.sum();
        FloatPixel actual = stats.average / average;
        if (euclidean_distance(actual, {0.255944, 0.582771, 0.161285}) > 0.2){
            return false;
        }
    }
    //    cout << "Start Checking bag" << endl;
    {
        ImageStats stats = image_stats(extract_box_reference(screen, m_menu_bag));
//        cout << stats.average << stats.stddev << endl;
        double stddev = stats.stddev.sum();
        if (stddev < 30 || stddev > 100){
            return false;
        }
        double average = stats.average.sum();
        FloatPixel actual = stats.average / average;
        if (euclidean_distance(actual, {0.485857, 0.414946, 0.099197}) > 0.2){
            return false;
        }
    }
    {
        ImageStats stats = image_stats(extract_box_reference(screen, m_menu_run));
//        cout << stats.average << stats.stddev << endl;
        double stddev = stats.stddev.sum();
        if (stddev < 30 || stddev > 100){
            return false;
        }
        double average = stats.average.sum();
        FloatPixel actual = stats.average / average;
        if (euclidean_distance(actual, {0.250977, 0.269868, 0.479155}) > 0.2){
            return false;
        }
    }

    return true;
}




BattleMenuWatcher::BattleMenuWatcher(BattleType battle_type, Color color)
    : BattleMenuDetector(battle_type, color)
    , VisualInferenceCallback("BattleMenuWatcher")
{}
void BattleMenuWatcher::make_overlays(VideoOverlaySet& items) const{
    BattleMenuDetector::make_overlays(items);
}
bool BattleMenuWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    //  Need 5 consecutive successful detections.
    if (!detect(frame)){
//        cout << "no detect" << endl;
        m_trigger_count = 0;
        return false;
    }
//    cout << "detected" << endl;
    m_trigger_count++;
    return m_trigger_count >= 5;
}




}
}
}
