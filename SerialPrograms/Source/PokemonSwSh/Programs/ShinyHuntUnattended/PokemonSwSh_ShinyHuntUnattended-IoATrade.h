/*  ShinyHuntUnattended-IoATrade
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedIoATrade_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedIoATrade_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
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

    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    TouchDateIntervalOption TOUCH_DATE_INTERVAL;

    TimeExpressionOption<uint16_t> START_TO_RUN_DELAY;
    SectionDividerOption m_advanced_options;
    TimeExpressionOption<uint16_t> FLY_DURATION;
    TimeExpressionOption<uint16_t> MOVE_DURATION;
    TimeExpressionOption<uint16_t> MASH_TO_TRADE_DELAY;
};


}
}
}
#endif
