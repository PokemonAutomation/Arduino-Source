/*  Shiny Hunt Autonomous - StrongSpawn
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousStrongSpawn_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousStrongSpawn_H

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


class ShinyHuntAutonomousStrongSpawn_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ShinyHuntAutonomousStrongSpawn_Descriptor();
};



class ShinyHuntAutonomousStrongSpawn : public SingleSwitchProgramInstance{
public:
    ShinyHuntAutonomousStrongSpawn(const ShinyHuntAutonomousStrongSpawn_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    StartInGripOrClosed START_IN_GRIP_MENU;
    GoHomeWhenDone GO_HOME_WHEN_DONE;
    TimeRollbackHours TIME_ROLLBACK_HOURS;

    Pokemon::EncounterBotLanguage LANGUAGE;
    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;

//    SectionDivider m_advanced_options;
};

}
}
}
#endif
