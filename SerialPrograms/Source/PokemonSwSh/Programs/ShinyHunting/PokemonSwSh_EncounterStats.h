/*  Encounter Stats
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EncounterStats_H
#define PokemonAutomation_PokemonSwSh_EncounterStats_H

#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class EncounterStats{
public:
    EncounterStats(bool shiny_types);
    virtual ~EncounterStats() = default;

    void log_stats(ProgramEnvironment& env, Logger& logger) const;
    virtual std::string stats() const;

    uint64_t encounters() const{ return m_encounters; }

    virtual std::string str_encounters() const;
    virtual std::string str_shinies() const;

    void add_non_shiny();
    void add_shiny();
    void add_star_shiny();
    void add_square_shiny();

protected:
    bool m_shiny_types;
    uint64_t m_encounters = 0;
    uint64_t m_shinies = 0;
    uint64_t m_star_shinies = 0;
    uint64_t m_square_shinies = 0;
};


}
}
}
#endif
