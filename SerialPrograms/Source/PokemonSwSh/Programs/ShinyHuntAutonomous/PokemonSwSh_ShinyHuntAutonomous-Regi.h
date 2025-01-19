/*  Shiny Hunt Autonomous - Regi
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousRegi_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousRegi_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"
#include "PokemonSwSh/Options/PokemonSwSh_EncounterBotCommon.h"
#include "PokemonSwSh/Options/PokemonSwSh_RegiSelector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;


class ShinyHuntAutonomousRegi_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntAutonomousRegi_Descriptor();

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class ShinyHuntAutonomousRegi : public SingleSwitchProgramInstance{
public:
    ShinyHuntAutonomousRegi();
    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    TouchDateIntervalOption TOUCH_DATE_INTERVAL;

    EncounterBotLanguage LANGUAGE;

    RegiSelectorOption REGI_NAME;
    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;

    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    TimeExpressionOption<uint16_t> EXIT_BATTLE_TIMEOUT;
    TimeExpressionOption<uint16_t> POST_BATTLE_MASH_TIME;
    TimeExpressionOption<uint16_t> TRANSITION_DELAY;
};

}
}
}
#endif
