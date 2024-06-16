/*  Item Printer Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_ItemPrinterTools_H
#define PokemonAutomation_PokemonSV_ItemPrinterTools_H

#include <array>
#include "CommonFramework/Language.h"

namespace PokemonAutomation{
    class AsyncDispatcher;
    class ConsoleHandle;
    class BotBaseContext;
namespace NintendoSwitch{
namespace PokemonSV{


enum class ItemPrinterJobs{
    Jobs_1      =   1,
    Jobs_5      =   5,
    Jobs_10     =   10,
};

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

enum class DistanceFromTarget{
    UNKNOWN,
    ON_TARGET,
    MINUS_1,
    MINUS_2,
    PLUS_1,
    PLUS_2,
};


void item_printer_start_print(
    ConsoleHandle& console, BotBaseContext& context,
    Language language, ItemPrinterJobs jobs
);
std::array<std::string, 10> item_printer_finish_print(
    AsyncDispatcher& dispatcher,
    ConsoleHandle& console, BotBaseContext& context,
    Language language
);




}
}
}
#endif
