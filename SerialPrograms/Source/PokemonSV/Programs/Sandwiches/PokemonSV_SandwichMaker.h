/*  Sandwich Maker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_SandwichMaker_H
#define PokemonAutomation_PokemonSV_SandwichMaker_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "PokemonSV/Options/PokemonSV_SandwichMakerOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class SandwichMaker_Descriptor : public SingleSwitchProgramDescriptor{
public:
    SandwichMaker_Descriptor();
};

class SandwichMaker : public SingleSwitchProgramInstance{
public:
    SandwichMaker();

    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    SandwichMakerOption SANDWICH_OPTIONS;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif



