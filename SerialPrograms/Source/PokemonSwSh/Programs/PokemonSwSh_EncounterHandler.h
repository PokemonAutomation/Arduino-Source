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
        const Pokemon::PokemonNameReader* name_reader, Language language,
        ShinyHuntTracker& session_stats,
        EncounterFilter& filter,
        bool video_on_shiny,
        EncounterBotNotificationLevel notification_level
    );


    //  Return true if program should stop.
    bool handle_standard_encounter(ShinyType shininess);
    bool handle_standard_encounter_runaway(ShinyType shininess, uint16_t exit_battle_time);


private:
    void update_frequencies(StandardEncounterDetection& encounter);
    void run_away_and_update_stats(
        StandardEncounterDetection& encounter,
        uint16_t exit_battle_time,
        ShinyType shininess
    );

private:
    const QString& m_program_name;
    ProgramEnvironment& m_env;
    ConsoleHandle& m_console;
    const Pokemon::PokemonNameReader* m_name_reader;
    const Language m_language;

    Pokemon::EncounterFrequencies m_frequencies;
    ShinyHuntTracker& m_session_stats;
    EncounterFilter& m_filter;
    const bool m_video_on_shiny;

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
