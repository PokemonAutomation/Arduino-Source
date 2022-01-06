/*  Encounter Handler
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EncounterHandler_H
#define PokemonAutomation_PokemonBDSP_EncounterHandler_H

#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "Pokemon/Pokemon_Notification.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonBDSP/Options/PokemonBDSP_EncounterBotCommon.h"
#include "PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinyEncounterDetector.h"
#include "PokemonBDSP/Programs/PokemonBDSP_EncounterDetection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class StandardEncounterHandler{
public:
    StandardEncounterHandler(
        ProgramEnvironment& env,
        ConsoleHandle& console,
        Language language,
        EncounterBotCommonOptions& settings,
        PokemonSwSh::ShinyHuntTracker& session_stats
    );

    //  Run away sequence for unexpected battle.
    void run_away_due_to_error(uint16_t exit_battle_time);

    //  Return true if program should stop.
    bool handle_standard_encounter(const DoublesShinyDetection& result);
    bool handle_standard_encounter_end_battle(const DoublesShinyDetection& result, uint16_t exit_battle_time);


private:
    std::vector<EncounterResult> results(StandardEncounterDetection& encounter);
    void update_frequencies(StandardEncounterDetection& encounter);
    void run_away_and_update_stats(
        StandardEncounterDetection& encounter,
        uint16_t exit_battle_time,
        const DoublesShinyDetection& result
    );

private:
    ProgramEnvironment& m_env;
    ConsoleHandle& m_console;
    bool m_enable_notifications;
    const Language m_language;
    EncounterBotCommonOptions& m_settings;

    EncounterFrequencies m_frequencies;
    PokemonSwSh::ShinyHuntTracker& m_session_stats;
    size_t m_consecutive_failures = 0;
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
