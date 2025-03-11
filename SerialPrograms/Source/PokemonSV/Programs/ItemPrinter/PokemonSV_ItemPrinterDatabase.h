/*  Item Printer Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_ItemPrinterDatabase_H
#define PokemonAutomation_PokemonSV_ItemPrinterDatabase_H

#include "Common/Cpp/Options/EnumDropdownDatabase.h"
#include "PokemonSV_ItemPrinterTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{
namespace ItemPrinter{



enum class PrebuiltOptions{
    NONE,
    ITEM_BONUS,
    BALL_BONUS,

    ABILITY_PATCH,
    EXP_CANDY,
    PP_MAX,

    ASSORTED_BALLS_1,
    ASSORTED_BALLS_2,
    ASSORTED_BALLS_3,

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
    WATER_TERA,
    STELLAR_TERA,
};
const EnumDropdownDatabase<PrebuiltOptions>& PrebuiltOptions_Database();

const EnumDropdownDatabase<PrebuiltOptions>& PrebuiltOptions_AutoMode_Database();

const EnumDropdownDatabase<PrebuiltOptions>& PrebuiltOptions_Simple_Database2();


struct ItemPrinterEnumOption{
    int64_t seed;
    PrebuiltOptions enum_value;
    ItemPrinterJobs jobs;
    uint16_t quantity_obtained;  // quantity obtained from the given seed, with given number of print jobs (ususally 5 print), with bonus active
};
const ItemPrinterEnumOption& option_lookup_by_enum(PrebuiltOptions enum_value);
const ItemPrinterEnumOption* option_lookup_by_seed(int64_t seed);



struct DateSeed{
    //  Seed details.
    int64_t seed;
    std::array<std::string, 10> regular;
    std::array<std::string, 10> item_bonus;
    std::array<std::string, 10> ball_bonus;

    DateSeed()
        : seed(0)
    {}

    DateSeed(
        int64_t p_seed,
        std::array<std::string, 10> p_items,
        std::array<std::string, 10> p_balls = {}
    )
        : seed(p_seed)
        , item_bonus(std::move(p_items))
        , ball_bonus(std::move(p_balls))
    {}
    DateSeed(
        int64_t p_seed,
        std::array<std::string, 10> p_regular,
        std::array<std::string, 10> p_item_bonus,
        std::array<std::string, 10> p_ball_bonus
    )
        : seed(p_seed)
        , regular(std::move(p_regular))
        , item_bonus(std::move(p_item_bonus))
        , ball_bonus(std::move(p_ball_bonus))
    {}

    operator bool() const{
        return seed >= 0;
    }
};
DateSeed get_date_seed(int64_t seed);


}
}
}
}
#endif
