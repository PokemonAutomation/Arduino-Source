/*  Max Lair AI Path Matchup
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "PokemonSwSh/Resources/PokemonSwSh_MaxLairDatabase.h"
#include "PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Pokemon.h"
#include "PokemonSwSh_MaxLair_AI_PathMatchup.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


struct PathMatchDatabase{
    std::map<PokemonType, std::set<std::string>> rentals_by_type;
    std::map<std::string, std::map<PokemonType, double>> type_vs_boss;

    static const PathMatchDatabase& instance(){
        static PathMatchDatabase database;
        return database;
    }

private:
    PathMatchDatabase(){
        std::string path = RESOURCE_PATH() + "PokemonSwSh/MaxLair/path_tree.json";
        JsonValue json = load_json_file(path);
        JsonObject& root = json.to_object_throw(path);

        {
            JsonObject& obj = root.get_object_throw("rental_by_type", path);
            for (const auto& type : POKEMON_TYPE_SLUGS()){
                if (type.first == PokemonType::NONE){
                    continue;
                }
                JsonArray& array = obj.get_array_throw(type.second, path);
                std::set<std::string>& set = rentals_by_type[type.first];
                for (auto& item : array){
                    std::string& str = item.to_string_throw(path);
                    set.insert(std::move(str));
                }
            }
        }

        JsonObject& node = root.get_object_throw("base_node", path).get_object_throw("hash_table");
        for (auto& item : node){
            std::map<PokemonType, double>& boss = type_vs_boss[item.first];

            JsonObject& obj = item.second.to_object_throw(path).get_object_throw("hash_table", path);

            for (const auto& type : POKEMON_TYPE_SLUGS()){
                if (type.first == PokemonType::NONE){
                    continue;
                }
                boss[type.first] = obj.get_double_throw(type.second, path);
            }
        }
    }


};


const std::set<std::string>& rentals_by_type(PokemonType type){
    const PathMatchDatabase& database = PathMatchDatabase::instance();
    auto iter = database.rentals_by_type.find(type);
    if (iter == database.rentals_by_type.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Type: " + std::to_string((int)type));
    }
    return iter->second;
}

double type_vs_boss(PokemonType type, const std::string& boss_slug){
    const PathMatchDatabase& database = PathMatchDatabase::instance();

    auto iter0 = database.type_vs_boss.find(boss_slug);
    if (iter0 == database.type_vs_boss.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Boss: " + boss_slug);
    }

    auto iter1 = iter0->second.find(type);
    if (iter1 == iter0->second.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Type: " + std::to_string((int)type));
    }

    return iter1->second;
}
double type_vs_boss(PokemonType type, PokemonType boss_type){
    using namespace papkmnlib;

    Type pkmnlib_type = serial_type_to_pkmnlib(boss_type);

    double weight = 0;
    size_t count = 0;
    for (const auto& item : all_bosses_by_dex()){
        const Pokemon& boss = get_pokemon(item.second);
        if (boss_type == PokemonType::NONE || boss.has_type(pkmnlib_type)){
            weight += type_vs_boss(type, boss.name());
            count++;
        }
    }

    return weight / (double)count;
}













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


template <typename Boss>
double evaluate_path(const Boss& boss, const std::vector<PathNode>& path){
    if (path.size() > 3){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Path is longer than 3: " + std::to_string(path.size()));
    }
    const double weights[] = {1, 2, 3};
    double weight = 0;

    size_t battle_index = 3 - path.size();
    size_t node_index = 0;
    for (; battle_index < 3; node_index++, battle_index++){
        weight += type_vs_boss(path[node_index].type, boss) * weights[battle_index];
    }
    return weight;
}


std::vector<PathNode> select_path(
    Logger* logger,
    const std::string& boss,
    const PathMap& pathmap, uint8_t wins, int8_t path_side
){
//    if (boss.empty() && state.path.boss == PokemonType::NONE){
//        logger.log("No information known about boss.", COLOR_ORANGE);
//        return {};
//    }

    std::vector<std::vector<PathNode>> paths = generate_paths(pathmap, wins, path_side);
    if (paths.empty()){
        if (logger){
            logger->log("No available paths due to read errors.", COLOR_RED);
        }
        return {};
    }

    std::multimap<double, std::vector<PathNode>, std::greater<double>> rank;
    if (boss.empty()){
        for (const std::vector<PathNode>& path : paths){
            rank.emplace(evaluate_path(pathmap.boss, path), path);
        }
    }else{
        for (const std::vector<PathNode>& path : paths){
            rank.emplace(evaluate_path(boss, path), path);
        }
    }
    std::string str = "Available Paths:\n";
    for (const auto& path : rank){
        str += std::to_string(path.first);
        str += " : ";
        str += dump_path(path.second);
        str += "\n";
    }
    if (logger){
        logger->log(str);
    }

    return std::move(rank.begin()->second);
}




}
}
}
}
