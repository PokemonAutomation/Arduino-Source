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
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh/Options/RegiSelector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class ShinyHuntUnattendedRegi : public SingleSwitchProgram{
public:
    ShinyHuntUnattendedRegi();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    TimeExpression<uint16_t> START_TO_RUN_DELAY;
    RegiSelector REGI_NAME;
    TimeExpression<uint32_t> TOUCH_DATE_INTERVAL;
    SectionDivider m_advanced_options;
    SimpleInteger<uint16_t> CORRECTION_INTERVAL;
    TimeExpression<uint16_t> TRANSITION_DELAY;
};

}
}
}
#endif
