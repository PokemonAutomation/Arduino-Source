/*  Under Attack Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonLA_UnderAttackDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


const char* UNDER_ATTACK_STRINGS[] = {
    "Unknown",
    "Safe",
    "Under Attack",
};


UnderAttackWatcher::UnderAttackWatcher(Logger& logger)
    : VisualInferenceCallback("UnderAttackWatcher")
    , m_logger(logger)
    , m_box(0.49, 0.07, 0.02, 0.03)
    , m_state(UnderAttackState::UNKNOWN)
{}

void UnderAttackWatcher::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_CYAN, m_box);
}
bool UnderAttackWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    UnderAttackState state = detect(frame);

    //  Clear out old history.
    WallClock threshold = timestamp - std::chrono::seconds(1);
    while (!m_history.empty()){
        Sample& sample = m_history.front();
        if (m_history.front().timestamp >= threshold){
            break;
        }
        m_counts[sample.state]--;
        m_history.pop_front();
    }

    size_t& count = m_counts[state];
    m_history.emplace_back(Sample{timestamp, state});
    count++;

    //  Return most reported state in the last window.
    UnderAttackState best_state = UnderAttackState::UNKNOWN;
    size_t best_count = 0;
    for (const auto& item : m_counts){
        if (best_count < item.second){
            best_count = item.second;
            best_state = item.first;
        }
    }

    UnderAttackState last_state = this->state();
    if (last_state != best_state){
        m_logger.log(
            std::string("State changed from ") + UNDER_ATTACK_STRINGS[(int)last_state] +
            " to " + UNDER_ATTACK_STRINGS[(int)best_state] + ".",
            COLOR_PURPLE
        );
    }
    m_state.store(best_state, std::memory_order_release);

    return false;
}


UnderAttackState UnderAttackWatcher::detect(const ImageViewRGB32& frame){
    ImageStats stats = image_stats(extract_box_reference(frame, m_box));
//    cout << stats.average << stats.stddev << endl;
//    if (stats.stddev.sum() > 100){
//        return UnderAttackState::SAFE;
//    }
    if (stats.average.r < 160 || stats.average.g > 170 || stats.average.b > 130){
        return UnderAttackState::SAFE;
    }
    if (stats.average.r < stats.average.g * 1.4 || stats.average.r < stats.average.g * 1.8){
        return UnderAttackState::SAFE;
    }
//    cout << "Under attack!" << endl;
    return UnderAttackState::UNDER_ATTACK;
}


}
}
}
