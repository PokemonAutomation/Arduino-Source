/*  Egg Power Sandwich Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSV_EggPowerSandwichOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



EggPowerSandwichOption::EggPowerSandwichOption()
    : GroupOption(
        "Egg Power Sandwich",
        LockWhileRunning::UNLOCKED,
        false, true
    )
    , MAX_NUM_SANDWICHES(
        "<b>Num Sandwiches:</b><br>How many sandwiches you can make before running out of ingredients.",
        LockWhileRunning::UNLOCKED,
        100, 0, 999
    )
    , EGG_SANDWICH_TYPE(
        "<b>Sandwich:</b><br>Which sandwich to get egg power.<br>"
        "<b>Great Peanut Butter Sandwich</b>: Use recipe No. 17. Must have enough ingredients to make it and ALL the other unlocked sandwich recipes for reliable recipe detection.<br>"
        "<b>Two Sweet/Sweet and Salty/Sweet and Bitter Herbs and Lettuce</b>: use the Lettuce and two Herbs. Must provide to program Herb locations on the condiments list.",
        {
            {EggSandwichType::GREAT_PEANUT_BUTTER, "great-peanut-butter", "Great Peanut Butter Sandwich"},
            {EggSandwichType::TWO_SWEET_HERBS, "two-sweet-herbs", "Two Sweet Herbs and Lettuce"},
            {EggSandwichType::SALTY_SWEET_HERBS, "salty-sweet-herbs", "Sweet and Salty Herbs and Lettuce"},
            {EggSandwichType::BITTER_SWEET_HERBS, "bitter-sweet-herbs", "Sweet and Bitter Herbs and Lettuce"},
        },
        LockWhileRunning::UNLOCKED,
        EggSandwichType::GREAT_PEANUT_BUTTER
    )
    , SWEET_HERB_INDEX_BACKWARDS(
        "<b>Sweet Herb Location:</b><br>If choosing Sweet Herb as sandwich ingredient, where the Sweet Herb is on the condiments list.",
        {
            {0, "0", "Last on list"},
            {1, "1", "2nd from last of the list"},
            {2, "2", "3rd from last of the list"},
            {3, "3", "4th from last of the list"},
            {4, "4", "5th from last of the list (the location if you have all types of herbs)"},
        },
        LockWhileRunning::UNLOCKED,
        4
    )
    , SALTY_HERB_INDEX_BACKWARDS(
        "<b>Salty Herb Location:</b><br>If choosing Salty Herb as sandwich ingredient, where the Salty Herb is on the condiments list.",
        {
            {0, "0", "Last on list"},
            {1, "1", "2nd from last of the list"},
            {2, "2", "3rd from last of the list"},
            {3, "3", "4th from last of the list (the location if you have all types of herbs)"},
        },
        LockWhileRunning::UNLOCKED,
        3
    )
    , BITTER_HERB_INDEX_BACKWARDS(
        "<b>Bitter Herb Location:</b><br>If choosing Bitter Herb as sandwich ingredient, where the Bitter Herb is on the condiments list.",
        {
            {0, "0", "Last on list"},
            {1, "1", "2nd from last of the list (the location if you have all types of herbs)"},
        },
        LockWhileRunning::UNLOCKED,
        1
    )
{
    PA_ADD_OPTION(MAX_NUM_SANDWICHES);
    PA_ADD_OPTION(EGG_SANDWICH_TYPE);
    PA_ADD_OPTION(SWEET_HERB_INDEX_BACKWARDS);
    PA_ADD_OPTION(SALTY_HERB_INDEX_BACKWARDS);
    PA_ADD_OPTION(BITTER_HERB_INDEX_BACKWARDS);
}


}
}
}
