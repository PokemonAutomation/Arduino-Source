/*  Encounter Handler
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EncounterHandler_H
#define PokemonAutomation_PokemonSwSh_EncounterHandler_H

#include "CommonFramework/Tools/ProgramEnvironment.h"
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
        const QString& program_name,
        ProgramEnvironment& env,
        ConsoleHandle& console,
        Language language,
        const EncounterBotCommonSettings& settings,
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
    const QString& m_program_name;
    ProgramEnvironment& m_env;
    ConsoleHandle& m_console;
    const Language m_language;
    const EncounterBotCommonSettings& m_settings;

    Pokemon::EncounterFrequencies m_frequencies;
    ShinyHuntTracker& m_session_stats;

    EncounterNotificationSender m_notification_sender;
};


void take_video(const BotBaseContext& context);
void run_away(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    uint16_t exit_battle_time
);


}
}
}
#endif
