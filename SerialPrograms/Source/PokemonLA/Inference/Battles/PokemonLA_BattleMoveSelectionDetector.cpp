/*  Battle Move Selection Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonLA_BattleMoveSelectionDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


BattleMoveSelectionDetector::BattleMoveSelectionDetector(Logger& logger, VideoOverlay& overlay, bool stop_on_detected)
    : VisualInferenceCallback("BattleMoveSelectionDetector")
    , m_stop_on_detected(stop_on_detected)
    , m_detected(false)
    , m_move_1_highlight(0.800, 0.6220, 0.02, 0.032)
    , m_move_2_highlight(0.779, 0.6875, 0.02, 0.032)
    , m_move_3_highlight(0.758, 0.7530, 0.02, 0.032)
    , m_move_4_highlight(0.737, 0.8185, 0.02, 0.032)
{}

void BattleMoveSelectionDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_BLUE, m_move_1_highlight);
    items.add(COLOR_BLUE, m_move_2_highlight);
    items.add(COLOR_BLUE, m_move_3_highlight);
    items.add(COLOR_BLUE, m_move_4_highlight);
}



bool BattleMoveSelectionDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    size_t highlighted = 0;

    const ImageStats move_1 = image_stats(extract_box_reference(frame, m_move_1_highlight));
    highlighted += is_white(move_1);
    // std::cout << "highlighted is now " << highlighted << std::endl;

    const ImageStats move_2 = image_stats(extract_box_reference(frame, m_move_2_highlight));
    highlighted += is_white(move_2);
    // std::cout << "highlighted is now " << highlighted << std::endl;
    if (highlighted > 1){
        m_detected.store(false, std::memory_order_release);
        return false;
    }
    
    const ImageStats move_3 = image_stats(extract_box_reference(frame, m_move_3_highlight));
    highlighted += is_white(move_3);
    // std::cout << "highlighted is now " << highlighted << std::endl;
    if (highlighted > 1){
        m_detected.store(false, std::memory_order_release);
        return false;
    }

    const ImageStats move_4 = image_stats(extract_box_reference(frame, m_move_4_highlight));
    highlighted += is_white(move_4);

    // std::cout << "move selection highlighted " << highlighted << std::endl;

    bool detected = highlighted == 1;
    m_detected.store(detected, std::memory_order_release);

    return detected && m_stop_on_detected;
}



}
}
}
