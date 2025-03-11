/*  Pokemon Berry Names
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_BerryNames_H
#define PokemonAutomation_Pokemon_BerryNames_H

#include <string>
#include <vector>

namespace PokemonAutomation{
namespace Pokemon{


class BerryNames{
public:
    const std::string& display_name() const{ return m_display_name; }

private:
    friend struct BerryNameDatabase;

    std::string m_display_name;
};


const BerryNames& get_berry_name(const std::string& slug);
const std::string& parse_berry_name(const std::string& display_name);
const std::string& parse_berry_name_nothrow(const std::string& display_name);

const std::vector<std::string>& BERRY_SLUGS();


}
}
#endif
