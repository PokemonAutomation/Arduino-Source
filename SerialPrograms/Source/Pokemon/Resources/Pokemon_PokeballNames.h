/*  Pokemon Pokeball Names
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_PokeballNames_H
#define PokemonAutomation_Pokemon_PokeballNames_H

#include <string>
#include <vector>
#include <map>
#include <QString>

namespace PokemonAutomation{
namespace Pokemon{


class PokeballNames{
public:
    const QString& display_name() const{ return m_display_name; }

private:
    friend struct PokeballNameDatabase;

    QString m_display_name;
};


const PokeballNames& get_pokeball_name(const std::string& slug);
const std::string& parse_pokeball_name(const QString& display_name);
const std::string& parse_pokeball_name_nothrow(const QString& display_name);

const std::vector<std::string>& POKEBALL_SLUGS();


}
}
#endif
