/*  Flying Trail Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_FlyingTrailFarmer_H
#define PokemonAutomation_PokemonSV_FlyingTrailFarmer_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class FlyingTrailFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    FlyingTrailFarmer_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;

};




class FlyingTrailFarmer : public SingleSwitchProgramInstance{
public:
    FlyingTrailFarmer();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    SimpleIntegerOption<uint8_t> NUM_TRIALS;
    EventNotificationsOption NOTIFICATIONS;
};



}
}
}
#endif
