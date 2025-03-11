/*  ShinyHuntUnattended-IoATrade
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedIoATrade_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedIoATrade_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ShinyHuntUnattendedIoATrade_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntUnattendedIoATrade_Descriptor();
};



class ShinyHuntUnattendedIoATrade : public SingleSwitchProgramInstance{
public:
    ShinyHuntUnattendedIoATrade();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    TouchDateIntervalOption TOUCH_DATE_INTERVAL;

    MillisecondsOption START_TO_RUN_DELAY0;
    SectionDividerOption m_advanced_options;
    MillisecondsOption FLY_DURATION0;
    MillisecondsOption MOVE_DURATION0;
    MillisecondsOption MASH_TO_TRADE_DELAY0;
};


}
}
}
#endif
