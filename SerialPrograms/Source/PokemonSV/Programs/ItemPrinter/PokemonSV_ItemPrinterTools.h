/*  Item Printer Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_ItemPrinterTools_H
#define PokemonAutomation_PokemonSV_ItemPrinterTools_H

#include <array>
#include "Common/Cpp/Options/EnumDropdownDatabase.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


enum class ItemPrinterJobs{
    Jobs_1      =   1,
    Jobs_5      =   5,
    Jobs_10     =   10,
};
const EnumDropdownDatabase<ItemPrinterJobs>& ItemPrinterJobs_Database();

struct ItemPrinterPrizeResult{
    std::array<std::string, 10> prizes;
    std::array<int16_t, 10> quantities;
};


void item_printer_start_print(
    VideoStream& stream, ProControllerContext& context,
    Language language, ItemPrinterJobs jobs
);
ItemPrinterPrizeResult item_printer_finish_print(
    VideoStream& stream, ProControllerContext& context,
    Language language
);




}
}
}
#endif
