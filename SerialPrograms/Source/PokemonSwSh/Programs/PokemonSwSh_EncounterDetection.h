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
#include "Pokemon/Pokemon_EncounterStats.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonSwSh/Options/PokemonSwSh_EncounterFilter.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class StandardEncounterDetection{
public:
    StandardEncounterDetection(
        ProgramEnvironment& env,
        ConsoleHandle& console,
        Language language,
        const EncounterFilter& filter,
        ShinyType shininess,
        std::chrono::milliseconds read_name_delay = std::chrono::milliseconds(500)
    );

    bool is_shiny() const;
    ShinyType shininess() const{ return m_shininess; }

    //  nullptr     =   disabled
    //  empty set   =   unable to detect
    const std::set<std::string>* candidates();

    std::pair<EncounterAction, std::string> get_action();

private:
    ProgramEnvironment& m_env;
    ConsoleHandle& m_console;

    const Language m_language;

    const EncounterFilter m_filter;
    const ShinyType m_shininess;
    const std::chrono::milliseconds m_read_name_delay;

    bool m_name_read = false;
    std::set<std::string> m_candidates;
};




}
}
}
#endif
