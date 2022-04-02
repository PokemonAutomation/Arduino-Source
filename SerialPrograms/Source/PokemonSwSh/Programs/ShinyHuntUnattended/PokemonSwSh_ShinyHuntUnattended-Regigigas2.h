/*  ShinyHuntUnattended-Regigigas2
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedRegigigas2_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedRegigigas2_H

#include "CommonFramework/Options/StaticTextOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ShinyHuntUnattendedRegigigas2_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ShinyHuntUnattendedRegigigas2_Descriptor();
};



class ShinyHuntUnattendedRegigigas2 : public SingleSwitchProgramInstance{
public:
    ShinyHuntUnattendedRegigigas2(const ShinyHuntUnattendedRegigigas2_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
    TouchDateIntervalOption TOUCH_DATE_INTERVAL;

    SimpleIntegerOption<uint8_t> REVERSAL_PP;
    TimeExpressionOption<uint16_t> START_TO_ATTACK_DELAY;
    SectionDividerOption m_advanced_options;
    TimeExpressionOption<uint16_t> ATTACK_TO_CATCH_DELAY;
    TimeExpressionOption<uint16_t> CATCH_TO_OVERWORLD_DELAY;
};

}
}
}
#endif
