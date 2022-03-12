/*  Shiny Hunt - Fixed Point
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_ShinyHuntFixedPoint_H
#define PokemonAutomation_PokemonLA_ShinyHuntFixedPoint_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/FloatingPointOption.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/PokemonLA_Locations.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class WarpLocationOption : public EnumDropdownOption{
public:
    WarpLocationOption();
    operator WarpSpot() const{
        return (WarpSpot)(size_t)*this;
    }
};




class ShinyHuntFixedPoint_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ShinyHuntFixedPoint_Descriptor();
};


class ShinyHuntFixedPoint : public SingleSwitchProgramInstance{
public:
    ShinyHuntFixedPoint(const ShinyHuntFixedPoint_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    void run_iteration(SingleSwitchProgramEnvironment& env);

private:
    class Stats;

    WarpLocationOption WARP_SPOT;

    SimpleIntegerOption<uint16_t> STOP_DISTANCE;
    FloatingPointOption FLAG_REACHED_DELAY;
    SimpleIntegerOption<uint64_t> NAVIGATION_TIMEOUT;

    ShinyDetectedActionOption SHINY_DETECTED;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
