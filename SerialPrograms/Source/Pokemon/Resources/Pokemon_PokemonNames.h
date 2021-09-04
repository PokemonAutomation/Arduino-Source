/*  Pokemon Names
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_PokemonNames_H
#define PokemonAutomation_Pokemon_PokemonNames_H

#include <vector>
#include <map>
#include <QString>
#include "CommonFramework/Language.h"

namespace PokemonAutomation{
namespace Pokemon{


class PokemonNames{
public:
    const QString& display_name() const{ return m_display_name; }
    const QString& display_name(Language language) const;

private:
    friend struct PokemonNameDatabase;

    QString m_display_name;
    std::map<Language, QString> m_display_names;
};


const PokemonNames& get_pokemon_name(const std::string& slug);
const std::string& parse_pokemon_name(const QString& display_name);
const std::string& parse_pokemon_name_nothrow(const QString& display_name);


}
}
#endif
