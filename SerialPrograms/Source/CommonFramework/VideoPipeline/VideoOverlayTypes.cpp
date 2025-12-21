/*  Video Overlay Types
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
//#include "Common/Cpp/Containers/Pimpl.tpp"
//#include "Common/Cpp/Concurrency/SpinLock.h"
#include "VideoOverlayTypes.h"

namespace PokemonAutomation{



#if 0
struct OverlayStat::Data{
    mutable SpinLock m_lock;
    OverlayStatSnapshot m_current;
};

OverlayStat::~OverlayStat() = default;
OverlayStat::OverlayStat()
    : m_data(CONSTRUCT_TOKEN)
{}

Color OverlayStat::get_text(std::string& text) const{
    ReadSpinLock lg(m_data->m_lock);
    text = m_data->m_current.text;
    return m_data->m_current.color;
}

void OverlayStat::set_text(std::string text, Color color){
    WriteSpinLock lg(m_data->m_lock);
    m_data->m_current.text = std::move(text);
    m_data->m_current.color = color;
}
#endif


OverlayStatUtilizationPrinter::OverlayStatUtilizationPrinter(double max_utilization)
    : m_max_utilization(max_utilization)
    , m_last_active(WallClock::min())
{}
OverlayStatSnapshot OverlayStatUtilizationPrinter::get_snapshot(const std::string& label, double utilization){
    WallClock now = current_time();
    bool active = utilization >= 0.01;

    if (!active &&
        (m_last_active == WallClock::min() || now - m_last_active > std::chrono::seconds(10))
    ){
        return OverlayStatSnapshot();
    }

    if (active){
        m_last_active = now;
    }

    Color color = COLOR_WHITE;
    if (utilization > 0.90 * m_max_utilization){
        color = COLOR_RED;
    }else if (utilization > 0.80 * m_max_utilization){
        color = COLOR_ORANGE;
    }else if (utilization > 0.50 * m_max_utilization){
        color = COLOR_YELLOW;
    }
    return OverlayStatSnapshot{
        label + " " + tostr_fixed(utilization * 100, 2) + " %",
        color
    };
}




}
