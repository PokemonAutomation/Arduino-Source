/*  ShinyHuntUnattended-Regigigas2
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedRegigigas2_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedRegigigas2_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ShinyHuntUnattendedRegigigas2_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntUnattendedRegigigas2_Descriptor();
};



class ShinyHuntUnattendedRegigigas2 : public SingleSwitchProgramInstance{
public:
    ShinyHuntUnattendedRegigigas2();

    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
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
