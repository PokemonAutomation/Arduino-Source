/*  ShinyHuntUnattended-Regi
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedRegi_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedRegi_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Options/StartInGripMenu.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"
#include "PokemonSwSh/Options/PokemonSwSh_RegiSelector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ShinyHuntUnattendedRegi_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ShinyHuntUnattendedRegi_Descriptor();
};



class ShinyHuntUnattendedRegi : public SingleSwitchProgramInstance{
public:
    ShinyHuntUnattendedRegi(const ShinyHuntUnattendedRegi_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    StartInGripOrGame START_IN_GRIP_MENU;
    TouchDateInterval TOUCH_DATE_INTERVAL;

    TimeExpression<uint16_t> START_TO_RUN_DELAY;
    RegiSelector REGI_NAME;
    SectionDivider m_advanced_options;
    SimpleInteger<uint16_t> CORRECTION_INTERVAL;
    TimeExpression<uint16_t> TRANSITION_DELAY;
};

}
}
}
#endif
