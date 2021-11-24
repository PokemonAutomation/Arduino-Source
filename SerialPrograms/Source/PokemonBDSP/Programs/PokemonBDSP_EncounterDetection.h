/*  Encounter Detection
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EncounterTracker_H
#define PokemonAutomation_PokemonBDSP_EncounterTracker_H

#include "CommonFramework/Language.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "Pokemon/Pokemon_EncounterStats.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonBDSP/Options/PokemonBDSP_EncounterFilter.h"
#include "PokemonBDSP/Inference/PokemonBDSP_ShinyEncounterDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class StandardEncounterDetection{
public:
    StandardEncounterDetection(
        ProgramEnvironment& env,
        ConsoleHandle& console,
        Language language,
        const EncounterFilterOption& filter,
        const DoublesShinyDetection& shininess,
        std::chrono::milliseconds read_name_delay = std::chrono::milliseconds(500)
    );

    bool is_double_battle() const{ return m_double_battle; }
    const std::set<std::string>* pokemon_left() const;
    const std::set<std::string>* pokemon_right() const;

    bool has_shiny() const;
    ShinyType overall_shininess() const{ return m_shininess.shiny_type; }
    ShinyType left_shininess() const{ return m_shininess_left; }
    ShinyType right_shininess() const{ return m_shininess_right; }

    std::pair<EncounterAction, std::string> get_action();

private:
    std::set<std::string> read_name(const QImage& screen, const ImageFloatBox& box);
    void run_overrides(
        std::vector<std::pair<EncounterAction, std::string>>& actions,
        const std::vector<EncounterFilterOverride>& overrides,
        const std::set<std::string>& pokemon, bool side_shiny
    ) const;

private:
    ProgramEnvironment& m_env;
    ConsoleHandle& m_console;

    const Language m_language;

    const EncounterFilterOption& m_filter;
    const DoublesShinyDetection& m_shininess;
    const std::chrono::milliseconds m_read_name_delay;

    bool m_double_battle = false;
    ShinyType m_shininess_left;
    ShinyType m_shininess_right;

    bool m_name_read = false;
    std::set<std::string> m_pokemon_left;
    std::set<std::string> m_pokemon_right;
};







}
}
}
#endif
