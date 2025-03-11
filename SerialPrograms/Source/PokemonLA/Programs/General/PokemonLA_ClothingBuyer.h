/*  Clothing Buyer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_ClothingBuyer_H
#define PokemonAutomation_PokemonLA_ClothingBuyer_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class ClothingBuyer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ClothingBuyer_Descriptor();
};


class ClothingBuyer : public SingleSwitchProgramInstance{
public:
    ClothingBuyer();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:

    BooleanCheckBoxOption CATEGORY_ROTATION;

};





}
}
}
#endif
