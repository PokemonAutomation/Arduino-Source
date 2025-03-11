/*  ShinyHuntUnattended-Regi
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedRegi_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedRegi_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"
#include "PokemonSwSh/Options/PokemonSwSh_RegiSelector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ShinyHuntUnattendedRegi_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntUnattendedRegi_Descriptor();
};



class ShinyHuntUnattendedRegi : public SingleSwitchProgramInstance{
public:
    ShinyHuntUnattendedRegi();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    TouchDateIntervalOption TOUCH_DATE_INTERVAL;

    MillisecondsOption START_TO_RUN_DELAY0;
    RegiSelectorOption REGI_NAME;
    SectionDividerOption m_advanced_options;
    SimpleIntegerOption<uint16_t> CORRECTION_INTERVAL;
    MillisecondsOption TRANSITION_DELAY0;
};

}
}
}
#endif
