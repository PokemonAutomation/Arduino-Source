/*  Pokemon Encounter Stats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_EncounterStats_H
#define PokemonAutomation_Pokemon_EncounterStats_H

#include <string>
#include <set>
#include <map>

namespace PokemonAutomation{
namespace Pokemon{


class PokemonEncounterSet{
public:
    PokemonEncounterSet() = default;
    PokemonEncounterSet(const std::set<std::string>& set)
        : m_set(set)
    {}
    PokemonEncounterSet(std::set<std::string>&& set)
        : m_set(std::move(set))
    {}

    friend bool operator<(const PokemonEncounterSet& x, const PokemonEncounterSet& y);

    std::string dump() const;


private:
    std::set<std::string> m_set;
};


class EncounterFrequencies{

public:
    void operator+=(const PokemonEncounterSet& set);

    bool empty() const{ return m_encounter_map.empty(); }
    std::string dump_sorted_map(const std::string& header) const;

public:
    using MapType = std::multimap<uint64_t, const PokemonEncounterSet*, std::greater<uint64_t>>;
    MapType to_sorted_map() const;

private:
    std::map<PokemonEncounterSet, uint64_t> m_encounter_map;
};


}
}
#endif
