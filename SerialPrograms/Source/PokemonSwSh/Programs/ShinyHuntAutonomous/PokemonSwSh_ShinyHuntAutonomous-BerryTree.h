/*  Shiny Hunt Autonomous - BerryTree
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousBerryTree_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousBerryTree_H

#include "CommonFramework/Options/SectionDivider.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Options/StartInGripMenu.h"
#include "NintendoSwitch/Options/GoHomeWhenDone.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonSwSh/Options/PokemonSwSh_EncounterBotCommon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ShinyHuntAutonomousBerryTree_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ShinyHuntAutonomousBerryTree_Descriptor();
};



class ShinyHuntAutonomousBerryTree : public SingleSwitchProgramInstance{
public:
    ShinyHuntAutonomousBerryTree(const ShinyHuntAutonomousBerryTree_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    StartInGripOrGame START_IN_GRIP_MENU;
    GoHomeWhenDone GO_HOME_WHEN_DONE;

    Pokemon::EncounterBotLanguage LANGUAGE;
    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;

    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> EXIT_BATTLE_TIMEOUT;
};

}
}
}
#endif
