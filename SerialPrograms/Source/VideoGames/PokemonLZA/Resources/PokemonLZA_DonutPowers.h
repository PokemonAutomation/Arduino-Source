/*  Donut Powers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_DonutPowers_H
#define PokemonAutomation_PokemonLZA_DonutPowers_H

#include <vector>
#include <map>
#include "CommonFramework/Language.h"
#include "CommonTools/Resources/SpriteDatabase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


const std::vector<std::string>& DONUT_POWERS_SLUGS();

class DonutPowers{
public:
    const std::string& display_name() const{ return m_display_name; }

private:
    friend struct PowerNameDatabase;

    std::string m_display_name;
    std::map<Language, std::string> m_display_names;
};

const DonutPowers& get_power_name(const std::string& slug);
const std::string& parse_power_name(const std::string& display_name);



const SpriteDatabase& DONUT_POWERS_DATABASE();


}
}
}
#endif
