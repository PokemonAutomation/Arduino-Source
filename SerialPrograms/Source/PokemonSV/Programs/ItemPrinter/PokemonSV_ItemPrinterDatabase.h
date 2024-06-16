/*  Item Printer Database
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_ItemPrinterDatabase_H
#define PokemonAutomation_PokemonSV_ItemPrinterDatabase_H

#include "Common/Cpp/EnumDatabase.h"
#include "PokemonSV_ItemPrinterTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



enum class ItemPrinterItems{
    NONE,
    ITEM_BONUS,
    BALL_BONUS,

    ABILITY_PATCH,
    EXP_CANDY,

    BUG_TERA,
    DARK_TERA,
    DRAGON_TERA,
    ELECTRIC_TERA,
    FAIRY_TERA,
    FIGHTING_TERA,
    FIRE_TERA,
    FLYING_TERA,
    GHOST_TERA,
    GRASS_TERA,
    GROUND_TERA,
    ICE_TERA,
    NORMAL_TERA,
    POISON_TERA,
    PSYCHIC_TERA,
    ROCK_TERA,
    STEEL_TERA,
    STELLAR_TERA,
    WATER_TERA,

    MASTER_BALL,
    FAST_BALL,
    FRIEND_BALL,
    LURE_BALL,
    LEVEL_BALL,
    HEAVY_BALL,
    LOVE_BALL,
    MOON_BALL,
    DREAM_BALL,
    SPORT_BALL,
    SAFARI_BALL,
    BEAST_BALL,

    ASSORTED_BALLS_1,
    ASSORTED_BALLS_2,
    ASSORTED_BALLS_3,
};


const EnumDatabase<ItemPrinterJobs>& ItemPrinterJobs_Database();
const EnumDatabase<ItemPrinterItems>& ItemPrinterItems_Database();


struct ItemPrinterEnumOption{
    int64_t seed;
    ItemPrinterItems enum_value;
    ItemPrinterJobs jobs;
};
const ItemPrinterEnumOption& option_lookup_by_enum(ItemPrinterItems enum_value);
const ItemPrinterEnumOption* option_lookup_by_seed(int64_t seed);



struct DateSeed{
    //  Seed details.
    int64_t seed;
    std::array<std::string, 10> items;
    std::array<std::string, 10> balls;

    DateSeed(
        int64_t p_seed,
        std::array<std::string, 10> p_items,
        std::array<std::string, 10> p_balls = {}
    )
        : seed(p_seed)
        , items(std::move(p_items))
        , balls(std::move(p_balls))
    {}

    operator bool() const{
        return seed >= 0;
    }
};
DateSeed get_date_seed(int64_t seed);


}
}
}
#endif
