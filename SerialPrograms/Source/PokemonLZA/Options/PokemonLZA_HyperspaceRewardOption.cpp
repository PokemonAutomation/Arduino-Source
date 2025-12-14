/*  Hyperspace Reward Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Logging/Logger.h"
#include "PokemonLZA/Resources/PokemonLZA_HyperspaceRewardNames.h"
#include "PokemonLZA_HyperspaceRewardOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

StringSelectDatabase make_hyperspace_reward_database(){
    StringSelectDatabase ret;
    for (const auto& slug : HYPERSPACE_REWARD_SLUGS()){
        const HyperspaceRewardNames& data = get_hyperspace_reward_name(slug);
        ret.add_entry(StringSelectEntry(slug, data.display_name()));
    }
    return ret;
}
const StringSelectDatabase& HYPERSPACE_REWARD_DATABASE(){
    static StringSelectDatabase database = make_hyperspace_reward_database();
    return database;
}


HyperspaceRewardCell::HyperspaceRewardCell(
    const std::string& default_slug
)
    : StringSelectCell(
        HYPERSPACE_REWARD_DATABASE(),
        LockMode::LOCK_WHILE_RUNNING,
        default_slug
    )
{}


}
}
}
