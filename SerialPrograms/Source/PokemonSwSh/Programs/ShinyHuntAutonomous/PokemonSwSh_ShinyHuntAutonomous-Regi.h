/*  Shiny Hunt Autonomous - Regi
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousRegi_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousRegi_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Options/StartInGripMenu.h"
#include "NintendoSwitch/Options/GoHomeWhenDone.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonSwSh/Options/EncounterFilter.h"
#include "PokemonSwSh/Options/RegiSelector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ShinyHuntAutonomousRegi_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ShinyHuntAutonomousRegi_Descriptor();
};



class ShinyHuntAutonomousRegi : public SingleSwitchProgramInstance{
public:
    ShinyHuntAutonomousRegi(const ShinyHuntAutonomousRegi_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    StartInGripOrGame START_IN_GRIP_MENU;
    GoHomeWhenDone GO_HOME_WHEN_DONE;

    Pokemon::PokemonNameReader m_name_reader;
    Pokemon::EncounterBotLanguage LANGUAGE;

    RegiSelector REGI_NAME;
    EncounterFilter FILTER;

    TimeExpression<uint32_t> TOUCH_DATE_INTERVAL;
    Pokemon::EncounterBotNotifications NOTIFICATION_LEVEL;

    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> EXIT_BATTLE_TIMEOUT;
    TimeExpression<uint16_t> POST_BATTLE_MASH_TIME;
    TimeExpression<uint16_t> TRANSITION_DELAY;
    BooleanCheckBox VIDEO_ON_SHINY;
};

}
}
}
#endif
