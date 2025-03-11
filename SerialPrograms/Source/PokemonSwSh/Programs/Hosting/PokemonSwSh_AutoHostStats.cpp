/*  Auto-Hosting Stats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSwSh_AutoHostStats.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



AutoHostStats::AutoHostStats()
    : m_raids(m_stats["Raids"])
    , m_timeouts(m_stats["Timeouts"])
    , m_empty(m_stats["Empty Raids"])
    , m_full(m_stats["Full Raids"])
    , m_total(m_stats["Total Raiders"])
{
    m_display_order.emplace_back("Raids");
    m_display_order.emplace_back("Timeouts");
    m_display_order.emplace_back("Empty Raids");
    m_display_order.emplace_back("Full Raids");
    m_display_order.emplace_back("Total Raiders");
}

void AutoHostStats::add_raid(size_t raiders){
    m_raids++;
    m_total += raiders;
    if (raiders == 0){
        m_empty++;
    }
    if (raiders == 3){
        m_full++;
    }
}
void AutoHostStats::add_timeout(){
    m_timeouts++;
}



}
}
}

