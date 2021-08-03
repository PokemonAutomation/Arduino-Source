/*  Mass Release
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MassRelease_H
#define PokemonAutomation_PokemonSwSh_MassRelease_H

#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/StartInGripMenu.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class MassRelease_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    MassRelease_Descriptor();
};



class MassRelease : public SingleSwitchProgramInstance{
public:
    MassRelease(const MassRelease_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    StartInGripOrGame START_IN_GRIP_MENU;
    SimpleInteger<uint8_t> BOXES_TO_RELEASE;
    BooleanCheckBox DODGE_SYSTEM_UPDATE_WINDOW;
};


}
}
}
#endif

