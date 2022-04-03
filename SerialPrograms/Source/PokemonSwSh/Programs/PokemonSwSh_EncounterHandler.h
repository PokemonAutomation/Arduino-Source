/*  Encounter Handler
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EncounterHandler_H
#define PokemonAutomation_PokemonSwSh_EncounterHandler_H

#include "CommonFramework/Tools/ConsoleHandle.h"
#include "Pokemon/Pokemon_Notification.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonSwSh/Options/PokemonSwSh_EncounterBotCommon.h"
#include "PokemonSwSh/Programs/PokemonSwSh_EncounterDetection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class StandardEncounterHandler{
public:
    StandardEncounterHandler(
        ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
        Language language,
        EncounterBotCommonOptions& settings,
        ShinyHuntTracker& session_stats
    );


    //  Return true if program should stop.
    bool handle_standard_encounter(const ShinyDetectionResult& result);
    bool handle_standard_encounter_end_battle(const ShinyDetectionResult& result, uint16_t exit_battle_time);


private:
    void update_frequencies(StandardEncounterDetection& encounter);
    void run_away_and_update_stats(
        StandardEncounterDetection& encounter,
        uint16_t exit_battle_time,
        const ShinyDetectionResult& result
    );

private:
    ProgramEnvironment& m_env;
    BotBaseContext& m_context;
    ConsoleHandle& m_console;
    const Language m_language;
    EncounterBotCommonOptions& m_settings;

    EncounterFrequencies m_frequencies;
    ShinyHuntTracker& m_session_stats;
    size_t m_consecutive_failures = 0;

//    EncounterNotificationSender m_notification_sender;
};


void take_video(BotBaseContext& context);
void run_away(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    uint16_t exit_battle_time
);


}
}
}
#endif
