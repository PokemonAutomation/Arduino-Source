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
    PokemonNameSelectData(const QString& json_file_slugs);
    const std::vector<std::pair<QString, QIcon>>& cases() const{ return m_list; }

protected:
    std::vector<std::pair<QString, QIcon>> m_list;
};


class PokemonNameSelect : private PokemonNameSelectData, public StringSelectOption{
public:
    PokemonNameSelect(
        QString label,
        const std::vector<std::string>& slugs,
        const std::string& default_slug = ""
    );
    PokemonNameSelect(
        QString label,
        const QString& json_file_slugs,
        const std::string& default_slug = ""
    );

    const std::string& slug() const;

private:
};


}
}
#endif
