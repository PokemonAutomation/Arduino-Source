/*  LGPE Fossil Revival
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLGPE_FossilRevival_H
#define PokemonAutomation_PokemonLGPE_FossilRevival_H

#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

class FossilRevival_Descriptor : public SingleSwitchProgramDescriptor{
public:
    FossilRevival_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class FossilRevival : public SingleSwitchProgramInstance{
public:
    FossilRevival();
    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    void run_revives(SingleSwitchProgramEnvironment& env, JoyconContext& context);

    IntegerEnumDropdownOption SLOT;
    SimpleIntegerOption<uint32_t> NUM_REVIVALS;
    
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif



