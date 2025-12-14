/*  Hyperspace Reward Names
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_HyperspaceRewardNames_H
#define PokemonAutomation_PokemonLZA_HyperspaceRewardNames_H

#include <string>
#include <vector>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


class HyperspaceRewardNames{
public:
    const std::string& display_name() const { return m_display_name; }

private:
    friend struct HyperspaceRewardNamesDatabase;

    std::string m_display_name;
};


const HyperspaceRewardNames& get_hyperspace_reward_name(const std::string& slug);
const std::string& parse_hyperspace_reward_name(const std::string& display_name);
const std::string& parse_hyperspace_reward_name_nothrow(const std::string& display_name);

const std::vector<std::string>& HYPERSPACE_REWARD_SLUGS();


}
}
}
#endif
