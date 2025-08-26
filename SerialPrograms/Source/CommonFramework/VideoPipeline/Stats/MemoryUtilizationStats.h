/*  Memory Utilization Stats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_MemoryUtilizationStats_H
#define PokemonAutomation_MemoryUtilizationStats_H

#include "CommonFramework/VideoPipeline/VideoOverlayTypes.h"

namespace PokemonAutomation{



class MemoryUtilizationStats;


class MemoryUtilizationStat : public OverlayStat{
public:
    MemoryUtilizationStat(MemoryUtilizationStats* parent)
        : m_parent(parent)
    {}
    virtual OverlayStatSnapshot get_current() override;

private:
    friend class MemoryUtilizationStats;

    MemoryUtilizationStats* m_parent;
    OverlayStatSnapshot m_snapshot;
};




class MemoryUtilizationStats{
public:
    MemoryUtilizationStats()
        : m_system(this)
        , m_process(this)
    {}

    void update();

private:
    static bool get_stat(
        std::string& stat_text,
        double& usage,
        std::string label,
        uint64_t total, uint64_t current
    );

private:
    friend class MemoryUtilizationStat;

public:
    MemoryUtilizationStat m_system;
    MemoryUtilizationStat m_process;
};





}
#endif
