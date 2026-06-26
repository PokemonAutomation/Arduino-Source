/*  FRLG Safari Optimal Action
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_SafariOptimalAction
#define PokemonAutomation_PokemonFRLG_SafariOptimalAction

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "CommonFramework/Language.h"
#include "PokemonFRLG/Inference/PokemonFRLG_BattleSelectionArrowDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
    class ConsoleHandle;
namespace PokemonFRLG{

static const std::set<std::string> SAFARI_ZONE_POKEMON_SUBSET = {
    "chansey", "doduo", "dragonair", "dratini", "exeggcute", "goldeen", "kangaskhan", "magikarp", "nidoran", "nidorina", "nidorino", "paras", "parasect", "pinsir", "poliwag", "psyduck", "rhyhorn", "scyther", "seaking", "slowpoke", "tauros", "venomoth", "venonat"
};

class SafariOptimalAction
{
public:
    SafariOptimalAction(Language game_language);

    // Returns the optimal action sequence for the given Pokemon and number of Safari Balls remaining.
    // Returns std::nullopt if no actions are available.
    std::optional<std::reference_wrapper<const std::vector<SafariBattleMenuOption>>> get_optimal_actions(ConsoleHandle& console, const std::string& pokemon_name, int balls_remaining) const;

private:
    std::map<std::string, std::vector<std::vector<SafariBattleMenuOption>>> m_action_table_by_pokemon;
};

}
}
}

#endif