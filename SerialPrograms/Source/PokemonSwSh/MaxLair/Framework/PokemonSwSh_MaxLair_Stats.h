/*  Max Lair Stats
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Stats_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Stats_H

#include "Common/Cpp/SpinLock.h"
#include "CommonFramework/Tools/StatsTracking.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


struct Stats : public StatsTracker{
    Stats()
        : m_runs(m_stats["Runs"])
        , m_errors(m_stats["Errors"])
        , m_wins(m_stats["Wins"])
        , m_catches(m_stats["Catches"])
        , m_shinies(m_stats["Shinies"])
        , m_shiny_legendary(m_stats["Shiny Legendary"])
    {
        m_display_order.emplace_back("Runs");
        m_display_order.emplace_back("Errors", true);
        m_display_order.emplace_back("Wins");
        m_display_order.emplace_back("Catches");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Shiny Legendary");
    }

    void add_run(size_t catches){
        SpinLockGuard lg(m_lock);
        m_runs++;
        m_catches += catches;
        if (catches >= 4){
            m_wins++;
        }
    }
    void add_error(){
        SpinLockGuard lg(m_lock);
        m_errors++;
    }
    void add_shiny(){
        SpinLockGuard lg(m_lock);
        m_shinies++;
    }
    void add_shiny_legendary(){
        SpinLockGuard lg(m_lock);
        m_shiny_legendary++;
    }


private:
    SpinLock m_lock;
    uint64_t& m_runs;
    uint64_t& m_errors;
    uint64_t& m_wins;
    uint64_t& m_catches;
    uint64_t& m_shinies;
    uint64_t& m_shiny_legendary;
};


}
}
}
}
#endif
