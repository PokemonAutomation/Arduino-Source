/*  Clothing Buyer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_ClothingBuyer_H
#define PokemonAutomation_PokemonLA_ClothingBuyer_H

#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class ClothingBuyer_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ClothingBuyer_Descriptor();
};


class ClothingBuyer : public SingleSwitchProgramInstance{
public:
    ClothingBuyer(const ClothingBuyer_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:

    BooleanCheckBoxOption CATEGORY_ROTATION;

};





}
}
}
#endif
