/*  Auto Fossil
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Hunt shiny and/or alpha fossils by repeated reviving fossils and check them in box
 */

#ifndef PokemonAutomation_PokemonLZA_AutoFossil_H
#define PokemonAutomation_PokemonLZA_AutoFossil_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/ButtonOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_MiscOptions.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


class AutoFossil_Descriptor : public SingleSwitchProgramDescriptor{
public:
    AutoFossil_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class AutoFossil : public SingleSwitchProgramInstance{
public:
    AutoFossil();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void revive_one_fossil(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    // return true if found a match
    bool check_fossils_in_one_box(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    PokemonLA::StopOnOption STOP_ON;

    SimpleIntegerOption<uint8_t> NUM_BOXES;
    EventNotificationOption FOUND_SHINY_OR_ALPHA;
    
    BooleanCheckBoxOption TAKE_VIDEO;
    BooleanCheckBoxOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif
