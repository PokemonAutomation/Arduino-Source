/*  Mass Release
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_MassRelease_H
#define PokemonAutomation_PokemonBDSP_MassRelease_H

#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class MassRelease_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    MassRelease_Descriptor();
};


class MassRelease : public SingleSwitchProgramInstance{
public:
    MassRelease(const MassRelease_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    struct Stats;

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    SimpleIntegerOption<uint8_t> BOXES_TO_RELEASE;
    EventNotificationsOption NOTIFICATIONS;
};



}
}
}
#endif
