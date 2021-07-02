/*  Encounter Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EncounterTracker_H
#define PokemonAutomation_PokemonSwSh_EncounterTracker_H

#include "CommonFramework/Language.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "Pokemon/Pokemon_NameReader.h"
#include "Pokemon/Pokemon_EncounterStats.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class StandardEncounterTracker{
public:
    StandardEncounterTracker(
        ShinyHuntTracker& stats,
        ProgramEnvironment& env,
        ConsoleHandle& console,
        Pokemon::PokemonNameReader* name_reader, Language language,
        bool require_square,
        uint16_t exit_battle_time,
        bool take_video,
        bool run_from_everything
    );

    virtual bool run_away(bool confirmed_encounter);

    bool process_result(ShinyDetection detection);

private:
    void take_video();
    void read_name();

protected:
    ShinyHuntTracker& m_shiny_stats_tracker;
    ProgramEnvironment& m_env;
    ConsoleHandle& m_console;
    Pokemon::PokemonNameReader* m_name_reader;
    Language m_language;
    bool m_require_square;
    uint16_t m_exit_battle_time;
    bool m_take_video;
    bool m_run_from_everything;

    Pokemon::PokemonEncounterStats m_encounter_stats;
};



}
}
}
#endif
