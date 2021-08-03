/*  Shiny Hunt Autonomous - IoATrade
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousIoATrade_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousIoATrade_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBox.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Options/StartInGripMenu.h"
#include "NintendoSwitch/Options/GoHomeWhenDone.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ShinyHuntAutonomousIoATrade_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ShinyHuntAutonomousIoATrade_Descriptor();
};



class ShinyHuntAutonomousIoATrade : public SingleSwitchProgramInstance{
public:
    ShinyHuntAutonomousIoATrade(const ShinyHuntAutonomousIoATrade_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    StartInGripOrGame START_IN_GRIP_MENU;
    GoHomeWhenDone GO_HOME_WHEN_DONE;

    TimeExpression<uint32_t> TOUCH_DATE_INTERVAL;
    Pokemon::EncounterBotNotifications NOTIFICATION_LEVEL;

    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> MASH_TO_TRADE_DELAY;
    BooleanCheckBox VIDEO_ON_SHINY;
    BooleanCheckBox RUN_FROM_EVERYTHING;
};


}
}
}
#endif
