/*  Pokemon Name Select
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_PokemonNameSelect_H
#define PokemonAutomation_Pokemon_PokemonNameSelect_H

#include "CommonTools/Options/StringSelectOption.h"

namespace PokemonAutomation{
namespace Pokemon{


struct PokemonNameSelectData{
    PokemonNameSelectData(const std::vector<std::string>& slugs);
    PokemonNameSelectData(const std::string& json_file_slugs);
    StringSelectDatabase m_database;
};


class PokemonNameSelectCell : private PokemonNameSelectData, public StringSelectCell{
public:
    PokemonNameSelectCell(
        const std::vector<std::string>& slugs,
        const std::string& default_slug = ""
    );
    PokemonNameSelectCell(
        const std::string& json_file_slugs,
        const std::string& default_slug = ""
    );
};

class PokemonNameSelectOption : private PokemonNameSelectData, public StringSelectOption{
public:
    PokemonNameSelectOption(
        std::string label,
        const std::vector<std::string>& slugs,
        const std::string& default_slug = ""
    );
    PokemonNameSelectOption(
        std::string label,
        const std::string& json_file_slugs,
        const std::string& default_slug = ""
    );
};



}
}
#endif
