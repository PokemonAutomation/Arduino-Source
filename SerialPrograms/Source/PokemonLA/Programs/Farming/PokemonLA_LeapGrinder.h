/*  Tree Leap Grinder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_LeapGrinder_H
#define PokemonAutomation_PokemonLA_LeapGrinder_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/StringSelectOption.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_MiscOptions.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

class LeapGrinder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    LeapGrinder_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class LeapGrinder : public SingleSwitchProgramInstance{
public:
    LeapGrinder();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    bool run_iteration(
        SingleSwitchProgramEnvironment& env, ProControllerContext& context,
        bool fresh_from_reset
    );
    bool quick_check(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

private:
    class RunRoute;

    OCR::LanguageOCROption LANGUAGE;

    StringSelectDatabase POKEMON_DATABASE;
    StringSelectOption POKEMON;

    SimpleIntegerOption<uint8_t> LEAPS;
    StopOnOption STOP_ON;
    ExitBattleMethodOption EXIT_METHOD;

    OverworldShinyDetectedActionOption SHINY_DETECTED_ENROUTE;
    EventNotificationOption FOUND_SHINY_OR_ALPHA;
    BattleMatchActionOption MATCH_DETECTED_OPTIONS;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
