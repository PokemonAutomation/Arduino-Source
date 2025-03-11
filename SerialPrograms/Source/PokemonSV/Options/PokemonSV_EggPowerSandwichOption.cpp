/*  Egg Power Sandwich Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSV_EggPowerSandwichOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



EggPowerSandwichOption::EggPowerSandwichOption()
    : GroupOption("Egg Power Sandwich", LockMode::UNLOCK_WHILE_RUNNING)
    , MAX_NUM_SANDWICHES(
        "<b>Max Sandwiches:</b><br>How many sandwiches you can make before running out of ingredients.",
        LockMode::UNLOCK_WHILE_RUNNING,
        100, 0, 999
    )
    , EGG_SANDWICH_TYPE(
        "<b>Sandwich Recipe:</b><br>Which sandwich to get egg power.<br>"
        "<b>Great Peanut Butter Sandwich</b>: Use recipe No. 17. Must have enough ingredients to make it and ALL the other unlocked sandwich recipes for reliable recipe detection.<br>"
        "<b>Two Sweet/Sweet and Salty/Sweet and Bitter Herbs and Lettuce</b>: use the Lettuce and two Herbs. Must provide Game Language option to read ingredient lists.",
        {
            {EggSandwichType::GREAT_PEANUT_BUTTER, "great-peanut-butter", "Great Peanut Butter Sandwich"},
            {EggSandwichType::TWO_SWEET_HERBS, "two-sweet-herbs", "Two Sweet Herbs and Lettuce"},
            {EggSandwichType::SALTY_SWEET_HERBS, "salty-sweet-herbs", "Sweet and Salty Herbs and Lettuce"},
            {EggSandwichType::BITTER_SWEET_HERBS, "bitter-sweet-herbs", "Sweet and Bitter Herbs and Lettuce"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        EggSandwichType::GREAT_PEANUT_BUTTER
    )
{
    PA_ADD_OPTION(MAX_NUM_SANDWICHES);
    PA_ADD_OPTION(EGG_SANDWICH_TYPE);
}





}
}
}
