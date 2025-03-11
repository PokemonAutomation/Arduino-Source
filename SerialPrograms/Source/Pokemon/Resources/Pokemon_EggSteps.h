/*  Egg Steps
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_EggSteps_H
#define PokemonAutomation_Pokemon_EggSteps_H

#include <map>
#include "CommonTools/Options/StringSelectOption.h"

namespace PokemonAutomation{

class SpriteDatabase;

namespace Pokemon{


class EggStepDatabase{
public:
    EggStepDatabase(const char* resource_path, const SpriteDatabase* sprites);

    size_t step_count(const std::string& slug) const;
    const StringSelectDatabase& database() const{
        return m_stringselect_database;
    }

private:
    std::map<std::string, uint16_t> m_slug_to_steps;
    StringSelectDatabase m_stringselect_database;
};




}
}
#endif
