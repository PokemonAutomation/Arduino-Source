/*  ShinyHuntUnattended-IoATrade
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedIoATrade_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedIoATrade_H

#include "CommonFramework/Options/SectionDivider.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Options/StartInGripMenu.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ShinyHuntUnattendedIoATrade_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ShinyHuntUnattendedIoATrade_Descriptor();
};



class ShinyHuntUnattendedIoATrade : public SingleSwitchProgramInstance{
public:
    ShinyHuntUnattendedIoATrade(const ShinyHuntUnattendedIoATrade_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    StartInGripOrGame START_IN_GRIP_MENU;
    TouchDateInterval TOUCH_DATE_INTERVAL;

    TimeExpression<uint16_t> START_TO_RUN_DELAY;
    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> FLY_DURATION;
    TimeExpression<uint16_t> MOVE_DURATION;
    TimeExpression<uint16_t> MASH_TO_TRADE_DELAY;
};


}
}
}
#endif
