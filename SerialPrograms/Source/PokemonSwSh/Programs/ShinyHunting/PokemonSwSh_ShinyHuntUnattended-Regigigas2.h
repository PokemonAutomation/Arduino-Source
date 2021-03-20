/*  ShinyHuntUnattended-Regigigas2
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedRegigigas2_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedRegigigas2_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class ShinyHuntUnattendedRegigigas2 : public SingleSwitchProgram{
public:
    ShinyHuntUnattendedRegigigas2();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    SimpleInteger<uint8_t> REVERSAL_PP;
    TimeExpression<uint16_t> START_TO_ATTACK_DELAY;
    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> ATTACK_TO_CATCH_DELAY;
    TimeExpression<uint16_t> CATCH_TO_OVERWORLD_DELAY;
    TimeExpression<uint32_t> TOUCH_DATE_INTERVAL;
};

}
}
}
#endif
