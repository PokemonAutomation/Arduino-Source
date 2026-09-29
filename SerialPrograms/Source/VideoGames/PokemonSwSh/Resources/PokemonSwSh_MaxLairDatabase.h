/*  Pokemon Sword/Shield Max Lair Rentals
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLairRentals_H
#define PokemonAutomation_PokemonSwSh_MaxLairRentals_H

#include <string>
#include <set>
#include <map>
#include "Pokemon/Pokemon_Types.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{

using namespace Pokemon;



//  Max Lair slug mappings with respect to name and sprite slugs.
struct MaxLairSlugs{
    std::string name_slug;
    std::set<std::string> sprite_slugs;
};
const std::map<std::string, MaxLairSlugs>& maxlair_slugs();
const MaxLairSlugs& get_maxlair_slugs(const std::string& slug);



struct MaxLairMove{
    std::string slug;
    MoveCategory category;
    PokemonType type;
    uint8_t base_power;
    double accuracy;
    uint8_t PP;
    bool spread;

    double correction_factor;
    double effective_power;
};

struct MaxLairMon{
    std::string species;
    PokemonType type[2];
    std::string ability;
    uint8_t base_stats[6];

    MaxLairMove moves[5];
    MaxLairMove max_moves[5];
};

const std::map<size_t, std::string>& all_bosses_by_dex();
bool is_boss(const std::string& slug);

const MaxLairMon& get_maxlair_mon(const std::string& slug);
const MaxLairMon* get_maxlair_mon_nothrow(const std::string& slug);




}
}
}
}
#endif
