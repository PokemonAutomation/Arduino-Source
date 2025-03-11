/*  Mass Release
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_MassRelease_H
#define PokemonAutomation_PokemonBDSP_MassRelease_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class MassRelease_Descriptor : public SingleSwitchProgramDescriptor{
public:
    MassRelease_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class MassRelease : public SingleSwitchProgramInstance{
public:
    MassRelease();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    SimpleIntegerOption<uint8_t> BOXES_TO_RELEASE;
    EventNotificationsOption NOTIFICATIONS;
};



}
}
}
#endif
