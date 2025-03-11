/*  ESP Training
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_ESPTraining_H
#define PokemonAutomation_PokemonSV_ESPTraining_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class ESPTraining_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ESPTraining_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class ESPTraining : public SingleSwitchProgramInstance{
public:
    ESPTraining();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    SimpleIntegerOption<uint32_t> ROUNDS;

    BooleanCheckBoxOption SAVE;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif
