/*  Pokemon Name Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_PokemonNameSelect_H
#define PokemonAutomation_Pokemon_PokemonNameSelect_H

#include "CommonFramework/Options/StringSelectOption.h"

namespace PokemonAutomation{
namespace Pokemon{


struct PokemonNameSelectData{
    PokemonNameSelectData(const std::vector<std::string>& slugs);
    PokemonNameSelectData(const std::string& json_file_slugs);
    const std::vector<std::pair<std::string, QIcon>>& cases() const{ return m_list; }

protected:
    std::vector<std::pair<std::string, QIcon>> m_list;
};


class PokemonNameSelect : private PokemonNameSelectData, public StringSelectOption{
public:
    PokemonNameSelect(
        std::string label,
        const std::vector<std::string>& slugs,
        const std::string& default_slug = ""
    );
    PokemonNameSelect(
        std::string label,
        const std::string& json_file_slugs,
        const std::string& default_slug = ""
    );

    const std::string& slug() const;

private:
};


}
}
#endif
