/*  Pokemon Encounter Stats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include "Common/Cpp/PrettyPrint.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "Pokemon_EncounterStats.h"

namespace PokemonAutomation{
namespace Pokemon{


bool operator<(const PokemonEncounterSet& x, const PokemonEncounterSet& y){
    auto iter0 = x.m_set.begin();
    auto iter1 = y.m_set.begin();
    while (true){
        bool end0 = iter0 == x.m_set.end();
        bool end1 = iter1 == y.m_set.end();
        if (end0 && end1){
            return false;
        }
        if (end0){
            return true;
        }
        if (end1){
            return false;
        }
        int cmp = strcmp(iter0->c_str(), iter1->c_str());
        if (cmp < 0){
            return true;
        }
        if (cmp > 0){
            return false;
        }

        ++iter0;
        ++iter1;
    }
}
std::string PokemonEncounterSet::dump() const{
    if (m_set.empty()){
        return "None - Unable to detect";
    }
    if (m_set.size() == 1){
        return get_pokemon_name(*m_set.begin()).display_name();
    }
    if (m_set.size() <= 5){
        std::string str = "Ambiguous (";
        bool first = true;
        for (const std::string& slug : m_set){
            if (!first){
                str += ", ";
            }
            first = false;
            str += get_pokemon_name(slug).display_name();
        }
        str += ")";
        return str;
    }
    return "Ambiguous (" + std::to_string(m_set.size()) + " candidates)";
}



void EncounterFrequencies::operator+=(const PokemonEncounterSet& set){
    m_encounter_map[set]++;
}

std::multimap<uint64_t, const PokemonEncounterSet*, std::greater<uint64_t>>
EncounterFrequencies::to_sorted_map() const{
    MapType ret;
    for (const auto& item : m_encounter_map){
        ret.emplace(item.second, &item.first);
    }
    return ret;
}
std::string EncounterFrequencies::dump_sorted_map(const std::string& header) const{
    MapType map = to_sorted_map();
    std::string str = header;
    for (const auto& item : map){
        str += tostr_u_commas(item.first);
        str += " : ";
        str += item.second->dump();
        str += "\n";
    }
    return str;
}



}
}

