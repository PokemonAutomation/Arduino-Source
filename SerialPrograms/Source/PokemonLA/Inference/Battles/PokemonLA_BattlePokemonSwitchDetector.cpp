/*  Battle Move Selection Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonLA/Inference/PokemonLA_CommonColorCheck.h"
#include "PokemonLA_BattlePokemonSwitchDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


BattlePokemonSwitchDetector::BattlePokemonSwitchDetector(Logger& logger, VideoOverlay& overlay, bool stop_on_detected)
    : VisualInferenceCallback("BattlePokemonSwitchDetector")
    , m_stop_on_detected(stop_on_detected)
    , m_detected(false)
    , m_white_bg_1(0.641, 0.178, 0.05, 0.023)
    , m_white_bg_2(0.641, 0.248, 0.05, 0.023)
    , m_white_bg_3(0.517, 0.195, 0.011, 0.061)
    , m_white_bg_4(0.924, 0.185, 0.019, 0.076)
    , m_ready_to_battle_bg_1(0.538, 0.216, 0.008, 0.018)
    , m_ready_to_battle_bg_2(0.686, 0.216, 0.008, 0.018)
    , m_button_plus_detector(logger, overlay, ButtonType::ButtonPlus, ImageFloatBox{0.044, 0.091, 0.043, 0.077}, std::chrono::milliseconds(0), false)
{}

void BattlePokemonSwitchDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_YELLOW, m_white_bg_1);
    items.add(COLOR_YELLOW, m_white_bg_2);
    items.add(COLOR_YELLOW, m_white_bg_3);
    items.add(COLOR_YELLOW, m_white_bg_4);
    items.add(COLOR_YELLOW, m_ready_to_battle_bg_1);
    items.add(COLOR_YELLOW, m_ready_to_battle_bg_2);
}


bool BattlePokemonSwitchDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    const ImageStats white_1 = image_stats(extract_box_reference(frame, m_white_bg_1));
    if(is_white(white_1, 500, 10) == false){
        // std::cout << "no white_1" << std::endl;
        m_detected.store(false, std::memory_order_release);
        return false;
    }

    const ImageStats white_2 = image_stats(extract_box_reference(frame, m_white_bg_2));
    if(is_white(white_2, 500, 10) == false){
        // std::cout << "no white_2" << std::endl;
        m_detected.store(false, std::memory_order_release);
        return false;
    }

    const ImageStats white_3 = image_stats(extract_box_reference(frame, m_white_bg_3));
    if(is_white(white_3, 500, 15) == false){
        // std::cout << "no white_3" << std::endl;
        m_detected.store(false, std::memory_order_release);
        return false;
    }

    const ImageStats white_4 = image_stats(extract_box_reference(frame, m_white_bg_4));
    if(is_white(white_4, 500, 10) == false){
        // std::cout << "no white_4" << std::endl;
        m_detected.store(false, std::memory_order_release);
        return false;
    }

    const ImageStats battle_1 = image_stats(extract_box_reference(frame, m_ready_to_battle_bg_1));
    if (!is_LA_dark_blue(battle_1)){
        // std::cout << "battle_1 not enough " << battle_1.average << " " << battle_1.stddev << std::endl;
        m_detected.store(false, std::memory_order_release);
        return false;
    }

    const ImageStats battle_2 = image_stats(extract_box_reference(frame, m_ready_to_battle_bg_2));
    if (!is_LA_dark_blue(battle_2)){
        // std::cout << "battle_2  not enough" << battle_2.average << " " << battle_2.stddev << std::endl;
        m_detected.store(false, std::memory_order_release);
        return false;
    }
    

    m_button_plus_detector.process_frame(frame, timestamp);
    bool detected = m_button_plus_detector.detected();
    // std::cout << "button plus detected " << detected << std::endl;
    m_detected.store(detected, std::memory_order_release);

    return detected && m_stop_on_detected;
}



}
}
}
