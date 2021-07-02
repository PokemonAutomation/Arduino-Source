/*  Clothing Buyer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ClothingBuyer_H
#define PokemonAutomation_PokemonSwSh_ClothingBuyer_H

#include "CommonFramework/Options/BooleanCheckBox.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ClothingBuyer_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ClothingBuyer_Descriptor();
};



class ClothingBuyer : public SingleSwitchProgramInstance{
public:
    ClothingBuyer(const ClothingBuyer_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    BooleanCheckBox CATEGORY_ROTATION;
};



}
}
}
#endif



