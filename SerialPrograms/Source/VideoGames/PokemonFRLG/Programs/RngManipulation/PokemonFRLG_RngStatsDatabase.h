/*  RNG Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_RngStatsDatabase_H
#define PokemonAutomation_PokemonFRLG_RngStatsDatabase_H

#include <map>
#include "Pokemon/Pokemon_AdvRng.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

using namespace Pokemon;

struct RngStats{
    BaseStats base_stats;
    int16_t gender_threshold;
    std::vector<std::string> abilities;
};

class RngStatsDatabase{
public:
    RngStatsDatabase(const char* json_path);

    static const RngStatsDatabase& instance();

    const RngStats& get_throw(const std::string& slug) const;
    const RngStats* get_nothrow(const std::string& slug) const;

public:
    using const_iterator = std::map<std::string, RngStats>::const_iterator;
    using       iterator = std::map<std::string, RngStats>::iterator;

    const_iterator cbegin   () const{ return m_database.cbegin(); }
    const_iterator begin    () const{ return m_database.begin(); }
          iterator begin    (){ return m_database.begin(); }
    const_iterator cend   () const{ return m_database.cend(); }
    const_iterator end    () const{ return m_database.end(); }
          iterator end    (){ return m_database.end(); }

private:
    std::map<std::string, RngStats> m_database;
};

// Returns the name of the ability in the given slot
std::string ability_name(const RngStats* stats, AdvAbility ability);

// Returns the gender implied by a PID's gender value
std::string gender_name(const RngStats* stats, uint8_t gender_value);



}
}
}
#endif
