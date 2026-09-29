/*  Max Lair Stats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Stats_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Stats_H

#include "CommonFramework/ProgramStats/StatsTracking.h"

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
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Wins");
        m_display_order.emplace_back("Catches");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Shiny Legendary", HIDDEN_IF_ZERO);
    }

    void add_run(size_t catches){
        m_runs++;
        m_catches += catches;
        if (catches >= 4){
            m_wins++;
        }
    }
    void add_error(){
        m_errors++;
    }
    void add_shiny(){
        m_shinies++;
    }
    void add_shiny_legendary(){
        m_shiny_legendary++;
    }


private:
    std::atomic<uint64_t>& m_runs;
    std::atomic<uint64_t>& m_errors;
    std::atomic<uint64_t>& m_wins;
    std::atomic<uint64_t>& m_catches;
    std::atomic<uint64_t>& m_shinies;
    std::atomic<uint64_t>& m_shiny_legendary;
};


class PathStats : public StatsTracker{
public:
    PathStats()
        : m_runs(m_stats["Runs"])
        , m_wins(m_stats["Wins"])
    {
        m_display_order.emplace_back("Runs");
        m_display_order.emplace_back("Wins");
    }

    void clear(){
//        cout << "PathStats::clear()" << endl;
        m_wins.store(0, std::memory_order_release);
        m_runs.store(0, std::memory_order_release);
    }
    void add_run(bool win){
//        cout << "PathStats::add_run(): " << win << endl;
        m_runs++;
        if (win){
            m_wins++;
        }
    }

    uint64_t runs() const{
//        cout << "PathStats::runs()" << m_runs << endl;
        return m_runs.load(std::memory_order_relaxed);
    }
    uint64_t wins() const{
        return m_wins.load(std::memory_order_relaxed);
    }
    double win_ratio() const{
        uint64_t runs = m_runs.load(std::memory_order_acquire);
        uint64_t wins = m_wins.load(std::memory_order_acquire);
        return (double)wins / (double)runs;
    }


private:
    std::atomic<uint64_t>& m_runs;
    std::atomic<uint64_t>& m_wins;
};



}
}
}
}
#endif
