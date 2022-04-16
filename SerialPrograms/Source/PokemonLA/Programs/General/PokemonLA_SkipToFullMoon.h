/*  Skip to Full Moon
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_SkipToFullMoon_H
#define PokemonAutomation_PokemonLA_SkipToFullMoon_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class SkipToFullMoon_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    SkipToFullMoon_Descriptor();
};


class SkipToFullMoon : public SingleSwitchProgramInstance{
public:
    SkipToFullMoon(const SkipToFullMoon_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:

    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
