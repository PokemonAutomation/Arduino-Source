/*  Clothing Buyer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ClothingBuyer_H
#define PokemonAutomation_PokemonSwSh_ClothingBuyer_H

#include "CommonFramework/Options/BooleanCheckBox.h"
#include "NintendoSwitch/Options/StartInGripMenu.h"
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
    StartInGripOrGame START_IN_GRIP_MENU;
    BooleanCheckBox CATEGORY_ROTATION;
};



}
}
}
#endif



