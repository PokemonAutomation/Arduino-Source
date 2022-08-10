/*  Clothing Buyer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ClothingBuyer_H
#define PokemonAutomation_PokemonSwSh_ClothingBuyer_H

#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ClothingBuyer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ClothingBuyer_Descriptor();
};



class ClothingBuyer : public SingleSwitchProgramInstance2{
public:
    ClothingBuyer();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
    BooleanCheckBoxOption CATEGORY_ROTATION;
};



}
}
}
#endif



