/*  ShinyHuntUnattended-SwordsOfJustice
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedSwordsOfJustice_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedSwordsOfJustice_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
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

    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
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
