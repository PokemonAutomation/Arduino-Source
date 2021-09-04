/*  Shiny Hunt Autonomous - Regigigas2
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousRegigigas2_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousRegigigas2_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Options/StartInGripMenu.h"
#include "NintendoSwitch/Options/GoHomeWhenDone.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"
#include "PokemonSwSh/Options/PokemonSwSh_EncounterBotCommon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ShinyHuntAutonomousRegigigas2_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ShinyHuntAutonomousRegigigas2_Descriptor();
};



class ShinyHuntAutonomousRegigigas2 : public SingleSwitchProgramInstance{
public:
    ShinyHuntAutonomousRegigigas2(const ShinyHuntAutonomousRegigigas2_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    bool kill_and_return(SingleSwitchProgramEnvironment& env) const;

private:
    StartInGripOrGame START_IN_GRIP_MENU;
    GoHomeWhenDone GO_HOME_WHEN_DONE;
    TouchDateInterval TOUCH_DATE_INTERVAL;

    SimpleInteger<uint8_t> REVERSAL_PP;
    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;

    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> CATCH_TO_OVERWORLD_DELAY;
};

}
}
}
#endif
