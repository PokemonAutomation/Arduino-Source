/*  Shiny Hunt Autonomous - Regigigas2
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousRegigigas2_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousRegigigas2_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Options/StartInGripMenu.h"
#include "NintendoSwitch/Options/GoHomeWhenDone.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonSwSh/Options/EncounterFilter.h"

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

    SimpleInteger<uint8_t> REVERSAL_PP;
    EncounterFilter FILTER;

    TimeExpression<uint32_t> TOUCH_DATE_INTERVAL;
    Pokemon::EncounterBotNotifications NOTIFICATION_LEVEL;

    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> CATCH_TO_OVERWORLD_DELAY;
    BooleanCheckBox VIDEO_ON_SHINY;
};

}
}
}
#endif
