/*  Egg Power Sandwich Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_EggPowerSandwichOption_H
#define PokemonAutomation_PokemonSV_EggPowerSandwichOption_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_SandwichRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class EggPowerSandwichOption : public GroupOption{
public:
    EggPowerSandwichOption();

    // Make sure herb indices are correct. If they are wrong, throw UserSetUpError.
    // void make_egg_sandwich()

public:
    SimpleIntegerOption<uint64_t> MAX_NUM_SANDWICHES;
    EnumDropdownOption<EggSandwichType> EGG_SANDWICH_TYPE;
};




}
}
}
#endif
