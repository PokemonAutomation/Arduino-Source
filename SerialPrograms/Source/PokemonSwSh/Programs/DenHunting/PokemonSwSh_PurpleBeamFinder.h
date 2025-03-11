/*  Purple Beam Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_PurpleBeamFinder_H
#define PokemonAutomation_PokemonSwSh_PurpleBeamFinder_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class PurpleBeamFinder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    PurpleBeamFinder_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class PurpleBeamFinder : public SingleSwitchProgramInstance{
public:
    PurpleBeamFinder();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    bool run(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

private:
    StartInGripOrGameOption START_LOCATION;

    EventNotificationOption NOTIFICATION_RED_BEAM;
    EventNotificationOption NOTIFICATION_PURPLE_BEAM;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    BooleanCheckBoxOption SAVE_SCREENSHOT;
    MillisecondsOption TIMEOUT_DELAY0;
//    FloatingPoint MAX_STDDEV;
    FloatingPointOption MIN_BRIGHTNESS;
    FloatingPointOption MIN_EUCLIDEAN;
    FloatingPointOption MIN_DELTA_STDDEV_RATIO;
    FloatingPointOption MIN_SIGMA_STDDEV_RATIO;
};


}
}
}
#endif

