/*  ShinyHuntUnattended-SwordsOfJustice
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedSwordsOfJustice_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedSwordsOfJustice_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class ShinyHuntUnattendedSwordsOfJustice : public SingleSwitchProgram{
public:
    ShinyHuntUnattendedSwordsOfJustice();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    TimeExpression<uint16_t> EXIT_CAMP_TO_RUN_DELAY;
    BooleanCheckBox AIRPLANE_MODE;
    SimpleInteger<uint8_t> TIME_ROLLBACK_HOURS;
    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> ENTER_CAMP_DELAY;
};

}
}
}
#endif
