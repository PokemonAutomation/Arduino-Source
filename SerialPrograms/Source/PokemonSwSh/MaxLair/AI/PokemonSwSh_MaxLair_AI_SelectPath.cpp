/*  Max Lair AI Select Path
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cstddef>
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"
#include "PokemonSwSh_MaxLair_AI_PathMatchup.h"

#include "PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Pokemon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{











//  0 for left-most path.
//  1 for 2nd from left.
//  2 ...
std::vector<PathNode> select_path(
    Logger& logger,
    const GlobalState& state,
    size_t player_index
){
    logger.log(
        "Player " + std::to_string(player_index) +
        ": Choosing a path... State =>\n" + state.dump(),
        COLOR_PURPLE
    );

    std::vector<std::vector<PathNode>> paths = generate_paths(state.path, state.wins, state.path_side);
    if (paths.empty()){
        return {};
    }

//    cout << "Paths = " << paths.size() << endl;
#if 0
    std::string str = "Available Paths:\n";
    for (const auto& path : paths){
        for (const auto& item : path){
            str += "[";
            str += std::to_string(item.path_slot);
            str += ":";
            str += get_type_slug(item.type);
            str += "] ";
        }
        str += "\n";
    }
    logger.log(str);

    //  Can't do anything since there's no path detection yet.
    return paths[random(0, (int)paths.size() - 1)][0].path_slot;
#endif

    std::vector<PathNode> path = select_path(
        &logger,
        state.boss,
        state.path, state.wins, state.path_side
    );
    if (path.empty()){
        return {};
    }
    return path;
}



}
}
}
}
