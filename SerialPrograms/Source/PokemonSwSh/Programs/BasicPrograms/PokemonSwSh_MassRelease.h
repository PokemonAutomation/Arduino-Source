/*  Mass Release
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MassRelease_H
#define PokemonAutomation_PokemonSwSh_MassRelease_H

#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class MassRelease : public SingleSwitchProgram{
public:
    MassRelease();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    SimpleInteger<uint8_t> BOXES_TO_RELEASE;
    BooleanCheckBox DODGE_SYSTEM_UPDATE_WINDOW;
};


}
}
}
#endif

