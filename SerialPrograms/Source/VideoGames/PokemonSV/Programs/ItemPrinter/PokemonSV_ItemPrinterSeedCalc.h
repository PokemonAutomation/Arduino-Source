/*  Item Printer Seed Calculation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Calculate Item Printer prizes from seed.
 *
 *  This file is ported from:
 *  https://github.com/kwsch/ItemPrinterDeGacha/blob/main/ItemPrinterDeGacha.Core/ItemPrinter.cs
 *
 */

#ifndef PokemonAutomation_PokemonSV_ItemPrinterSeedCalc_H
#define PokemonAutomation_PokemonSV_ItemPrinterSeedCalc_H

#include "PokemonSV_ItemPrinterDatabase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{
namespace ItemPrinter{


DateSeed calculate_seed_prizes(int64_t seed);


}
}
}
}
#endif
