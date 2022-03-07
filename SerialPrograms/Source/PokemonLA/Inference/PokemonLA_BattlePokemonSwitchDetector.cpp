/*  Battle Move Selection Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "PokemonLA_BattlePokemonSwitchDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


BattlePokemonSwitchDetector::BattlePokemonSwitchDetector(LoggerQt& logger, VideoOverlay& overlay, bool stop_on_detected)
    : VisualInferenceCallback("BattlePokemonSwitchDetector")
    , m_stop_on_detected(stop_on_detected)
    , m_detected(false)
    , m_white_bg_1(0.541, 0.251, 0.15, 0.026)
    , m_white_bg_2(0.541, 0.171, 0.15, 0.026)
    , m_white_bg_3(0.517, 0.195, 0.011, 0.061)
    , m_white_bg_4(0.924, 0.185, 0.019, 0.076)
    , m_ready_to_battle_bg_1(0.54, 0.216, 0.016, 0.018)
    , m_ready_to_battle_bg_2(0.676, 0.216, 0.016, 0.018)
{}

void BattlePokemonSwitchDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_YELLOW, m_white_bg_1);
    items.add(COLOR_YELLOW, m_white_bg_2);
    items.add(COLOR_YELLOW, m_white_bg_3);
    items.add(COLOR_YELLOW, m_white_bg_4);
    items.add(COLOR_YELLOW, m_ready_to_battle_bg_1);
    items.add(COLOR_YELLOW, m_ready_to_battle_bg_2);
}


bool BattlePokemonSwitchDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    size_t highlighted = 0;

    const ImageStats white_1 = image_stats(extract_box(frame, m_white_bg_1));
    if(is_solid(white_1, {0.34,0.34,0.34}, 0.15) == false){
        m_detected.store(false, std::memory_order_release);
        return false;
    }

    const ImageStats white_2 = image_stats(extract_box(frame, m_white_bg_2));
    if(is_solid(white_2, {0.34,0.34,0.34}, 0.15) == false){
        m_detected.store(false, std::memory_order_release);
        return false;
    }

    const ImageStats white_3 = image_stats(extract_box(frame, m_white_bg_3));
    if(is_solid(white_3, {0.34,0.34,0.34}, 0.15) == false){
        m_detected.store(false, std::memory_order_release);
        return false;
    }

    const ImageStats white_4 = image_stats(extract_box(frame, m_white_bg_4));
    if(is_solid(white_4, {0.34,0.34,0.34}, 0.15) == false){
        m_detected.store(false, std::memory_order_release);
        return false;
    }

    const ImageStats battle_1 = image_stats(extract_box(frame, m_ready_to_battle_bg_1));
    if(is_solid(battle_1, {0.216,0.338,0.446}, 0.15) == false){
        m_detected.store(false, std::memory_order_release);
        return false;
    }

    const ImageStats battle_2 = image_stats(extract_box(frame, m_ready_to_battle_bg_2));
    const bool detected = is_solid(battle_2, {0.216,0.338,0.446}, 0.15);
    m_detected.store(detected, std::memory_order_release);

    return detected && m_stop_on_detected;
}



}
}
}
