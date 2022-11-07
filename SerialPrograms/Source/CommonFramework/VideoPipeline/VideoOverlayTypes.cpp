/*  Video Overlay Types
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "VideoOverlayTypes.h"

namespace PokemonAutomation{




struct OverlayStat::Data{
    mutable SpinLock m_lock;
    OverlayStatSnapshot m_current;
};

OverlayStat::~OverlayStat() = default;
OverlayStat::OverlayStat()
    : m_data(CONSTRUCT_TOKEN)
{}

Color OverlayStat::get_text(std::string& text) const{
    SpinLockGuard lg(m_data->m_lock);
    text = m_data->m_current.text;
    return m_data->m_current.color;
}

void OverlayStat::set_text(std::string text, Color color){
    SpinLockGuard lg(m_data->m_lock);
    m_data->m_current.text = std::move(text);
    m_data->m_current.color = color;
}




}
