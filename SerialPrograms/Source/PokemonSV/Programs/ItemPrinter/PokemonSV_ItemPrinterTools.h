/*  Item Printer Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_ItemPrinterTools_H
#define PokemonAutomation_PokemonSV_ItemPrinterTools_H

#include <array>
#include "CommonFramework/Language.h"
#include "Common/Cpp/EnumDatabase.h"

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
const EnumDatabase<ItemPrinterJobs>& ItemPrinterJobs_Database();


void item_printer_start_print(
    AsyncDispatcher& dispatcher,
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
