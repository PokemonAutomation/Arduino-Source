/*  Encounters Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_EncountersDatabase_H
#define PokemonAutomation_PokemonFRLG_EncountersDatabase_H

#include <map>
#include "Pokemon/Pokemon_AdvRng.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

using namespace Pokemon;

class EncountersDatabase{
public:
    EncountersDatabase(const char* json_path);
    const std::map<std::string, std::vector<AdvEncounterSlot>>& get_throw(const std::string& slug) const;
    const std::map<std::string, std::vector<AdvEncounterSlot>>* get_nothrow(const std::string& slug) const;

public:
    using const_iterator = std::map<std::string, std::map<std::string, std::vector<AdvEncounterSlot>>>::const_iterator;
    using       iterator = std::map<std::string, std::map<std::string, std::vector<AdvEncounterSlot>>>::iterator;

    const_iterator cbegin   () const{ return m_database.cbegin(); }
    const_iterator begin    () const{ return m_database.begin(); }
          iterator begin    (){ return m_database.begin(); }
    const_iterator cend   () const{ return m_database.cend(); }
    const_iterator end    () const{ return m_database.end(); }
          iterator end    (){ return m_database.end(); }

private:
    std::map<std::string, std::map<std::string, std::vector<AdvEncounterSlot>>> m_database;
};



}
}
}
#endif
