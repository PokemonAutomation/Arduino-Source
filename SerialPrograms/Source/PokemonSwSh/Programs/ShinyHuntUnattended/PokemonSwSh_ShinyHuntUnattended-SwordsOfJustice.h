/*  ShinyHuntUnattended-SwordsOfJustice
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedSwordsOfJustice_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedSwordsOfJustice_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ShinyHuntUnattendedSwordsOfJustice_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntUnattendedSwordsOfJustice_Descriptor();
};



class ShinyHuntUnattendedSwordsOfJustice : public SingleSwitchProgramInstance{
public:
    ShinyHuntUnattendedSwordsOfJustice();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    TimeRollbackHoursOption TIME_ROLLBACK_HOURS;

    MillisecondsOption EXIT_CAMP_TO_RUN_DELAY0;
    BooleanCheckBoxOption AIRPLANE_MODE;
    SectionDividerOption m_advanced_options;
    MillisecondsOption ENTER_CAMP_DELAY0;
};

}
}
}
#endif
