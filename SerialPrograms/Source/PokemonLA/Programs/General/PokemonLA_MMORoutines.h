/*  Pokemon LA MMO Routines
 *
 *  Functions to run MMO related tasks
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include <set>
#include <string>

namespace PokemonAutomation{
namespace NintendoSwitch{

class SingleSwitchProgramEnvironment;

namespace PokemonLA{

// The name of each MMO event happening at each region. Their slugs are:
// - "fieldlands-mmo"
// - "mirelands-mmo"
// - "coastlands-mmo"
// - "highlands-mmo"
// - "icelands-mmo"
const std::vector<std::string>& MMO_NAMES();


// After we find an MMO on map specified by `mmo_name`,
// start from Jubilife Village gate, with camera facing towards the village, run towards the gate
// and travel to the region with that MMO. Go to Mai to show all MMO pokemon on map and read their
// sprites and star symbols.
// The function returns when the map opened by Mai is shown with all pokemon revealed.
// Return true if an MMO pokemon matches slugs in `desired_MMOs`, or if an MMO pokemon with a star
// matches slugs in `desired_star_MMOs`.
// - mmo_name: MMO event slug, e.g. "fieldlands-mmo"
// - num_mmo_found: update this value by adding how many MMO pokemon found during this function.
// - num_star_mmo_found: update this value by adding how many MMO pokemon with star symbol found.
std::set<std::string> enter_region_and_read_MMO(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    const std::string& mmo_name,
    const std::set<std::string>& desired_MMOs,
    const std::set<std::string>& desired_star_MMOs,
    bool debug,
    int& num_mmo_found,
    int& num_star_mmo_found,
    bool fresh_from_reset
);


}
}
}
