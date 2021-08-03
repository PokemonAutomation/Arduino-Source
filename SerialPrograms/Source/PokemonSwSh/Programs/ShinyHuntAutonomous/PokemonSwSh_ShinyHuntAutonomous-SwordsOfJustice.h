/*  Shiny Hunt Autonomous - Swords Of Justice
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousSwordsOfJustice_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousSwordsOfJustice_H

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


class ShinyHuntAutonomousSwordsOfJustice_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ShinyHuntAutonomousSwordsOfJustice_Descriptor();
};



class ShinyHuntAutonomousSwordsOfJustice : public SingleSwitchProgramInstance{
public:
    ShinyHuntAutonomousSwordsOfJustice(const ShinyHuntAutonomousSwordsOfJustice_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    StartInGripOrGame START_IN_GRIP_MENU;
    GoHomeWhenDone GO_HOME_WHEN_DONE;

    Pokemon::PokemonNameReader m_name_reader;
    Pokemon::EncounterBotLanguage LANGUAGE;

    BooleanCheckBox AIRPLANE_MODE;
    EncounterFilter FILTER;

    SimpleInteger<uint8_t> TIME_ROLLBACK_HOURS;
    Pokemon::EncounterBotNotifications NOTIFICATION_LEVEL;

    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> EXIT_BATTLE_TIMEOUT;
    TimeExpression<uint16_t> POST_BATTLE_MASH_TIME;
    TimeExpression<uint16_t> ENTER_CAMP_DELAY;
    BooleanCheckBox VIDEO_ON_SHINY;
};

}
}
}
#endif
