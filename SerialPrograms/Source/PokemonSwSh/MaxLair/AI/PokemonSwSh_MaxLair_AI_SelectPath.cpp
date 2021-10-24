/*  Max Lair AI Select Path
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


struct PathNode{
    uint8_t path_slot;
    PokemonType type;
};
std::vector<std::vector<PathNode>> generate_paths(
    const PathMap& map, uint8_t wins, int8_t side
);


void append_subpath(
    std::vector<std::vector<PathNode>>& paths,
    const PathNode& node,
    const std::vector<std::vector<PathNode>>& subpaths
){
    for (const auto& item : subpaths){
        paths.emplace_back();
        std::vector<PathNode>& path = paths.back();
        path.emplace_back(node);
        path.insert(path.end(), item.begin(), item.end());
    }
}
std::vector<std::vector<PathNode>> generate_paths(
    const PathMap& map, uint8_t wins, int8_t side
){
    std::vector<std::vector<PathNode>> ret;

    if (wins == 0){
        append_subpath(ret, {0, map.mon1[0]}, generate_paths(map, 1, 0));
        append_subpath(ret, {1, map.mon1[1]}, generate_paths(map, 1, 1));
        return ret;
    }

    if (side == -1){
        return ret;
    }

    if (wins == 1){
        std::vector<std::vector<PathNode>> left = generate_paths(map, 2, 0);
        std::vector<std::vector<PathNode>> right = generate_paths(map, 2, 1);
        switch (map.path_type){
        case 0:
            if (side == 0){
                append_subpath(ret, {0, map.mon2[0]}, left);
                append_subpath(ret, {1, map.mon2[1]}, left);
                append_subpath(ret, {2, map.mon2[2]}, left);
            }else{
                append_subpath(ret, {0, map.mon2[2]}, left);
                append_subpath(ret, {1, map.mon2[3]}, right);
            }
            break;
        case 1:
            if (side == 0){
                append_subpath(ret, {0, map.mon2[0]}, left);
                append_subpath(ret, {1, map.mon2[1]}, left);
            }else{
                append_subpath(ret, {0, map.mon2[1]}, left);
                append_subpath(ret, {1, map.mon2[2]}, right);
                append_subpath(ret, {2, map.mon2[3]}, right);
            }
            break;
        case 2:
            if (side == 0){
                append_subpath(ret, {0, map.mon2[0]}, left);
                append_subpath(ret, {1, map.mon2[1]}, right);
            }else{
                append_subpath(ret, {0, map.mon2[2]}, right);
                append_subpath(ret, {1, map.mon2[3]}, right);
            }
            break;
        }
        return ret;
    }

    if (wins == 2){
        switch (map.path_type){
        case 0:
            if (side == 0){
                ret.emplace_back(std::vector<PathNode>{PathNode{0, map.mon3[0]}});
                ret.emplace_back(std::vector<PathNode>{PathNode{1, map.mon3[1]}});
            }else{
                ret.emplace_back(std::vector<PathNode>{PathNode{0, map.mon3[1]}});
                ret.emplace_back(std::vector<PathNode>{PathNode{1, map.mon3[2]}});
                ret.emplace_back(std::vector<PathNode>{PathNode{2, map.mon3[3]}});
            }
            break;
        case 1:
            if (side == 0){
                ret.emplace_back(std::vector<PathNode>{PathNode{0, map.mon3[0]}});
                ret.emplace_back(std::vector<PathNode>{PathNode{1, map.mon3[1]}});
                ret.emplace_back(std::vector<PathNode>{PathNode{2, map.mon3[2]}});
            }else{
                ret.emplace_back(std::vector<PathNode>{PathNode{0, map.mon3[2]}});
                ret.emplace_back(std::vector<PathNode>{PathNode{1, map.mon3[3]}});
            }
            break;
        case 2:
            if (side == 0){
                ret.emplace_back(std::vector<PathNode>{PathNode{0, map.mon3[0]}});
                ret.emplace_back(std::vector<PathNode>{PathNode{1, map.mon3[1]}});
            }else{
                ret.emplace_back(std::vector<PathNode>{PathNode{0, map.mon3[1]}});
                ret.emplace_back(std::vector<PathNode>{PathNode{1, map.mon3[2]}});
                ret.emplace_back(std::vector<PathNode>{PathNode{2, map.mon3[3]}});
            }
            break;
        }
        return ret;
    }

    return ret;
}






//  0 for left-most path.
//  1 for 2nd from left.
//  2 ...
uint8_t select_path(
    Logger& logger,
    const GlobalState& state,
    size_t player_index
){
    logger.log(
        "Player " + std::to_string(player_index) +
        ": Choosing a path... State =>\n" + state.dump(),
        "purple"
    );

    std::vector<std::vector<PathNode>> paths = generate_paths(state.path, state.wins, state.path_side);
    if (paths.empty()){
        logger.log("No available paths due to read errors. Picking left-most path.", Qt::red);
        return 0;
    }

//    cout << "Paths = " << paths.size() << endl;
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
}



}
}
}
}
