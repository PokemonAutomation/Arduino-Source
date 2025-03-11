/*  Skip to Full Moon
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_SkipToFullMoon_H
#define PokemonAutomation_PokemonLA_SkipToFullMoon_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class SkipToFullMoon_Descriptor : public SingleSwitchProgramDescriptor{
public:
    SkipToFullMoon_Descriptor();
};


class SkipToFullMoon : public SingleSwitchProgramInstance{
public:
    SkipToFullMoon();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:

    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
