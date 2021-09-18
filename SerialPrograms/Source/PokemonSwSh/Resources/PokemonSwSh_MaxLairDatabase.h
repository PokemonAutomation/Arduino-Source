/*  Pokemon Sword/Shield Max Lair Rentals
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLairRentals_H
#define PokemonAutomation_PokemonSwSh_MaxLairRentals_H

#include <string>
#include "PokemonSwSh_TypeMatchup.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

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


const MaxLairMon& get_maxlair_mon(const std::string& slug);
const MaxLairMon* get_maxlair_mon_nothrow(const std::string& slug);





}
}
}
#endif
