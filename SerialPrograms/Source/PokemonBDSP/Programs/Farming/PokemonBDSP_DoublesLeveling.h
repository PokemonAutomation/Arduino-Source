/*  Double Battle Leveling
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_DoublesLeveling_H
#define PokemonAutomation_PokemonBDSP_DoublesLeveling_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonBDSP/Options/PokemonBDSP_LearnMove.h"
#include "PokemonBDSP/Options/PokemonBDSP_EncounterBotCommon.h"
#include "PokemonBDSP/Programs/PokemonBDSP_OverworldTrigger.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class DoublesLeveling_Descriptor : public SingleSwitchProgramDescriptor{
public:
    DoublesLeveling_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class DoublesLeveling : public SingleSwitchProgramInstance{
public:
    DoublesLeveling();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;


private:
    bool battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    Pokemon::EncounterBotLanguage LANGUAGE;

    OverworldTrigger TRIGGER_METHOD;
    OnLearnMoveOption ON_LEARN_MOVE;

    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;

    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
//    MillisecondsOption WATCHDOG_TIMER;
    MillisecondsOption EXIT_BATTLE_TIMEOUT0;
};




}
}
}
#endif
