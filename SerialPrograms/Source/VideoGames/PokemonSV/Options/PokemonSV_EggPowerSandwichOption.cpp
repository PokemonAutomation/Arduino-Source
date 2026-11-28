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
        "<b>Sandwich Recipe:</b><br>Which sandwich to get egg power. Higher Egg Power finds eggs faster.<br>"
        "<b>Best recipe I can make</b>: tries Great Peanut Butter (No. 17), Great Marmalade (No. 25), Peanut Butter (No. 16), "
        "then Jam (No. 12), and makes the first one you have unlocked and have ingredients for. Use this if you are not sure.<br>"
        "<b>Recipe sandwiches (No. 12, 16, 17, 25)</b>: must have unlocked the recipe and have enough ingredients to make it "
        "and ALL the other unlocked sandwich recipes for reliable recipe detection. Jam is available early; "
        "the Peanut Butter recipes unlock after 4 Gym Badges.<br>"
        "<b>Two Sweet/Sweet and Salty/Sweet and Bitter Herbs and Lettuce</b>: Egg Power Lv 3. Needs Herba Mystica from 5- and 6-star raids. "
        "Use the Lettuce and two Herbs. Must provide Game Language option to read ingredient lists.",
        {
            {EggSandwichType::BEST_AVAILABLE_RECIPE, "best-available-recipe", "Best recipe I can make (Egg Power Lv 1-2)"},
            {EggSandwichType::GREAT_PEANUT_BUTTER, "great-peanut-butter", "Lv 2: Great Peanut Butter Sandwich (No. 17)"},
            {EggSandwichType::GREAT_MARMALADE, "great-marmalade", "Lv 2: Great Marmalade Sandwich (No. 25)"},
            {EggSandwichType::PEANUT_BUTTER, "peanut-butter", "Lv 1: Peanut Butter Sandwich (No. 16)"},
            {EggSandwichType::JAM, "jam", "Lv 1: Jam Sandwich (No. 12)"},
            {EggSandwichType::TWO_SWEET_HERBS, "two-sweet-herbs", "Lv 3: Two Sweet Herbs and Lettuce"},
            {EggSandwichType::SALTY_SWEET_HERBS, "salty-sweet-herbs", "Lv 3: Sweet and Salty Herbs and Lettuce"},
            {EggSandwichType::BITTER_SWEET_HERBS, "bitter-sweet-herbs", "Lv 3: Sweet and Bitter Herbs and Lettuce"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        EggSandwichType::BEST_AVAILABLE_RECIPE
    )
{
    PA_ADD_OPTION(MAX_NUM_SANDWICHES);
    PA_ADD_OPTION(EGG_SANDWICH_TYPE);
}





}
}
}
