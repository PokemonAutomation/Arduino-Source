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

class ClothingBuyer : public SingleSwitchProgram{
public:
    ClothingBuyer();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    BooleanCheckBox CATEGORY_ROTATION;
};



}
}
}
#endif



