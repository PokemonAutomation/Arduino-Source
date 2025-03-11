/*  Encounter Handler
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EncounterHandler_H
#define PokemonAutomation_PokemonBDSP_EncounterHandler_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonBDSP/Options/PokemonBDSP_EncounterBotCommon.h"
#include "PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinyEncounterDetector.h"
#include "PokemonBDSP/Programs/PokemonBDSP_EncounterDetection.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonBDSP{

//  Handle wild encounters using feedback.
class StandardEncounterHandler{
public:
    StandardEncounterHandler(
        ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
        Language language,
        EncounterBotCommonOptions& settings,
        PokemonSwSh::ShinyHuntTracker& session_stats
    );

    //  Run away sequence for unexpected battle.
    //  Must be called during a battle. Hit "Run" and go through the dialogue until
    //  end of battle is detected.
    //  The pokemon must be able to run successfully with no trapping or run away failure.
    //
    //  exit_battle_time: number of ticks to wait for battle ends after pressing "Run" button.
    //    If end of battle not detected in time, log the error but don't throw exception.
    void run_away_due_to_error(Milliseconds exit_battle_time);

    //  Use shiny detection result and inference of pokemon species to determine whether
    //  to stop the program according to the user setting.
    //  Return true if program should stop.
    bool handle_standard_encounter(const DoublesShinyDetection& result);
    //  Use shiny detection result and inference of pokemon species to determine whether
    //  to stop the program, run away from battle or catch the pokemon according to the user
    //  setting.
    //  Return true if program should stop.
    bool handle_standard_encounter_end_battle(const DoublesShinyDetection& result, Milliseconds exit_battle_time);


private:
    std::vector<EncounterResult> results(StandardEncounterDetection& encounter);
    // Record the types of pokemon encountered into encounter stats.
    // The stats are then logged.
    void update_frequencies(StandardEncounterDetection& encounter);

private:
    ProgramEnvironment& m_env;
    ProControllerContext& m_context;
    VideoStream& m_stream;
    const Language m_language;
    EncounterBotCommonOptions& m_settings;

    EncounterFrequencies m_frequencies;
    PokemonSwSh::ShinyHuntTracker& m_session_stats;
    size_t m_consecutive_failures = 0;
};


void take_video(ProControllerContext& context);
void run_away(
    ProgramEnvironment& env,
    VideoStream& stream,
    Milliseconds exit_battle_time
);



class LeadingShinyTracker{
public:
    LeadingShinyTracker(Logger& logger);

    void report_result(ShinyType type);

private:
    Logger& m_logger;
    size_t m_consecutive_shinies;
};




}
}
}
#endif
