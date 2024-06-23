/*  Encounter Detection
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EncounterTracker_H
#define PokemonAutomation_PokemonSwSh_EncounterTracker_H

#include "CommonFramework/Language.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "Pokemon/Pokemon_EncounterStats.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonSwSh/Options/EncounterFilter/PokemonSwSh_EncounterFilterOption.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonSwSh{


class StandardEncounterDetection{
public:
    StandardEncounterDetection(
        ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
        Language language,
        const EncounterFilterOption2& filter,
        ShinyType shininess,
        std::chrono::milliseconds read_name_delay = std::chrono::milliseconds(500)
    );

    bool is_shiny() const;
    ShinyType shininess() const{ return m_shininess; }

    //  nullptr     =   disabled
    //  empty set   =   unable to detect
    const std::set<std::string>* candidates();

    EncounterActionFull get_action();

private:
    ProgramEnvironment& m_env;
    ConsoleHandle& m_console;
    BotBaseContext& m_context;

    const Language m_language;

    const EncounterFilterOption2& m_filter;
    const ShinyType m_shininess;
    const std::chrono::milliseconds m_read_name_delay;

    bool m_name_read = false;
    std::set<std::string> m_candidates;
};




}
}
}
#endif
