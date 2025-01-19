/*  ShinyHuntUnattended-Regi
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedRegi_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntUnattendedRegi_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
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

    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    TouchDateIntervalOption TOUCH_DATE_INTERVAL;

    TimeExpressionOption<uint16_t> START_TO_RUN_DELAY;
    RegiSelectorOption REGI_NAME;
    SectionDividerOption m_advanced_options;
    SimpleIntegerOption<uint16_t> CORRECTION_INTERVAL;
    TimeExpressionOption<uint16_t> TRANSITION_DELAY;
};

}
}
}
#endif
