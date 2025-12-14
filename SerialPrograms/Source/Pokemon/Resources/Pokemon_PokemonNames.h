/*  Pokemon Names
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_PokemonNames_H
#define PokemonAutomation_Pokemon_PokemonNames_H

#include <string>
#include <vector>
#include <map>
#include "CommonFramework/Language.h"

namespace PokemonAutomation{
namespace Pokemon{


// Store a pokemon's display name for a pokemon species.
// It does not differentiate any form differences. The total different PokemonNames
// equals the highest available national dex number.
class PokemonNames{
public:
    const std::string& display_name() const{ return m_display_name; }
    const std::string& display_name(Language language) const;

public:
    const static std::string NULL_SLUG;
private:
    friend struct PokemonNameDatabase;

    std::string m_display_name;
    std::map<Language, std::string> m_display_names;
};


// From pokemon slug to its display name.
// The slug is for pokemon species. Each slug corresponds to one national dex number.
// Throw InternalProgramError if the given slug is not found in the database.
const PokemonNames& get_pokemon_name(const std::string& slug);
// From pokemon slug to its display name.
// The slug is for pokemon species. Each slug corresponds to one national dex number.
// Return nullptr if the given slug is not found in the database.
const PokemonNames* get_pokemon_name_nothrow(const std::string& slug);
// From a pokemon English display name to its slug.
// The display name is for pokemon species. Each name corresponds to one national dex number.
// Throw InternalProgramError if the given name is not found in the database.
const std::string& parse_pokemon_name(const std::string& display_name);
// From a pokemon English display name to its slug.
// The display name is for pokemon species. Each name corresponds to one national dex number.
// Return nullptr if the given name is not found in the database.
const std::string& parse_pokemon_name_nothrow(const std::string& display_name);

// Get max national dex number from the database.
size_t max_national_dex_number();
// Get pokemon slug from national dex number. Throw InternalProgramError if no such dex number.
const std::string& get_pokemon_from_national_dex_number(size_t national_id);


// Load a list of pokemon name slugs from a json file. May throw file error.
// json_path: the path relative to the resource root, accessed via 
//   RESOURCE_PATH() declared in CommonFramework/Globals.h.
std::vector<std::string> load_pokemon_slug_json_list(const char* json_path);


}
}
#endif
