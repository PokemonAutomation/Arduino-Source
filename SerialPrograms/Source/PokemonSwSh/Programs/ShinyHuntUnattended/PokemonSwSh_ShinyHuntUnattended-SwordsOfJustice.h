/*  ShinyHuntUnattended-SwordsOfJustice
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedSwordsOfJustice_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedSwordsOfJustice_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ShinyHuntUnattendedSwordsOfJustice_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ShinyHuntUnattendedSwordsOfJustice_Descriptor();
};



class ShinyHuntUnattendedSwordsOfJustice : public SingleSwitchProgramInstance{
public:
    ShinyHuntUnattendedSwordsOfJustice(const ShinyHuntUnattendedSwordsOfJustice_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
    TimeRollbackHoursOption TIME_ROLLBACK_HOURS;

    TimeExpressionOption<uint16_t> EXIT_CAMP_TO_RUN_DELAY;
    BooleanCheckBoxOption AIRPLANE_MODE;
    SectionDividerOption m_advanced_options;
    TimeExpressionOption<uint16_t> ENTER_CAMP_DELAY;
};

}
}
}
#endif
