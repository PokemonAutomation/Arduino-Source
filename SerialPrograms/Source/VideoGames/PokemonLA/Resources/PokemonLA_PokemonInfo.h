/*  Pokemon LA Info
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_PokemonInfo_H
#define PokemonAutomation_PokemonLA_PokemonInfo_H

#include <string>
#include <set>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


enum class Gender{
    Unknown,
    Genderless,
    Male,
    Female,
};
inline const char* get_gender_str(Gender gender){
    switch (gender){
    case Gender::Unknown:       return "Unknown";
    case Gender::Genderless:    return "Genderless";
    case Gender::Male:          return "Male";
    case Gender::Female:        return "Female";
    }
    return nullptr;
}


struct PokemonDetails{
    std::set<std::string> name_candidates;
    Gender gender = Gender::Unknown;
    bool is_shiny = false;
    bool is_alpha = false;
};




}
}
}
#endif
