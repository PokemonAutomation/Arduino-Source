/*  Let's Go HP Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <vector>
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Pokemon/Inference/Pokemon_ReadHpBar.h"
#include "PokemonSV_LetsGoHpReader.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



LetsGoHpWatcher::LetsGoHpWatcher(Color color)
    : VisualInferenceCallback("LetsGoHpWatcher")
    , m_color(color)
    , m_box(0.057, 0.930, 0.065, 0.010)
//    , m_last_known_value(-1)
{}

void LetsGoHpWatcher::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}


void LetsGoHpWatcher::clear(){
//    m_last_known_value.store(-1, std::memory_order_relaxed);
    WriteSpinLock lg(m_lock);
    m_history.clear();
}
double LetsGoHpWatcher::last_known_value() const{
    std::vector<double> sorted;
    {
        ReadSpinLock lg(m_lock);

        //  No history.
        if (m_history.empty()){
            return -1;
        }

        //  Insufficient history.
        if (m_history.size() < 10){
            return -1;
        }

        //  Make local copy.
        for (auto& item : m_history){
            sorted.emplace_back(item.second);
        }
    }

    //  Sort and return median.
    std::sort(sorted.begin(), sorted.end());

    return sorted[sorted.size() / 2];

//    return m_last_known_value.load(std::memory_order_relaxed);
}


bool LetsGoHpWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    double hp = read_hp_bar(
        extract_box_reference(frame, ImageFloatBox(0.055, 0.928, 0.067, 0.012))
    );
    if (hp <= 0){
        return false;
    }
//    m_last_known_value.store(hp, std::memory_order_relaxed);

    WriteSpinLock lg(m_lock);
    if (m_history.empty()){
        m_history.emplace_back(timestamp, hp);
        return false;
    }
    if (timestamp < m_history.back().first){
        global_logger_tagged().log("LetsGoHpWatcher(): Detected that time has traveled backwards. Clearing history.", COLOR_RED);
        m_history.clear();
        m_history.emplace_back(timestamp, hp);
        return false;
    }

    //  Don't let spam skew the results.
    if (timestamp < m_history.back().first + std::chrono::milliseconds(40)){
        return false;
    }

    while (m_history.size() > 20){
        m_history.pop_front();
    }
    m_history.emplace_back(timestamp, hp);

    return false;
}






}
}
}
