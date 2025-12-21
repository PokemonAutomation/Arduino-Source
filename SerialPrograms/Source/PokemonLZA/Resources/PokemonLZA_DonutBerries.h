/*  Donut Berries
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_Berries_H
#define PokemonAutomation_PokemonLZA_Berries_H

#include <vector>
#include <map>
#include "CommonFramework/Language.h"
#include "CommonTools/Resources/SpriteDatabase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


const std::vector<std::string>& DONUT_BERRIES_SLUGS();

class DonutBerries{
public:
    const std::string& display_name() const{ return m_display_name; }

private:
    friend struct BerryNameDatabase;

    std::string m_display_name;
    std::map<Language, std::string> m_display_names;
};

const DonutBerries& get_berry_name(const std::string& slug);
const std::string& parse_berry_name(const std::string& display_name);



const SpriteDatabase& DONUT_BERRIES_DATABASE();


}
}
}
#endif
