/*  FRLG Safari Optimal Action
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "CommonFramework/Globals.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonFRLG/Programs/PokemonFRLG_SafariOptimalAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

namespace{

SafariBattleMenuOption parse_safari_action(const JsonValue& json, const std::string& path){
    int64_t value = json.to_integer_throw(path);
    switch (value){
    case 0:
        return SafariBattleMenuOption::BALL;
    case 1:
        return SafariBattleMenuOption::BAIT;
    case 2:
        return SafariBattleMenuOption::ROCK;
    case 3:
        return SafariBattleMenuOption::RUN;
    default:
        throw JsonParseException(path, "Invalid SafariBattleMenuOption value: " + std::to_string(value));
    }
}

}

SafariOptimalAction::SafariOptimalAction(Language game_language){
    std::string language_folder = game_language == Language::Japanese ? "Japanese/" : "International/";
    std::string path = RESOURCE_PATH() + "PokemonFRLG/SafariOptimalAction/" + language_folder;

    for (const std::string& pokemon_name : SAFARI_ZONE_POKEMON_SUBSET){
        std::string json_path = path + pokemon_name + ".json";
        JsonValue json = load_json_file(json_path);
        JsonArray& root = json.to_array_throw(json_path);

        std::vector<std::vector<SafariBattleMenuOption>>& action_table = m_action_table_by_pokemon[pokemon_name];
        action_table.reserve(root.size());

        for (const JsonValue& row_value : root){
            const JsonArray& row = row_value.to_array_throw(json_path);
            std::vector<SafariBattleMenuOption>& actions = action_table.emplace_back();
            actions.reserve(row.size());

            for (const JsonValue& action_value : row){
                actions.emplace_back(parse_safari_action(action_value, json_path));
            }
        }
    }
}

std::optional<std::reference_wrapper<const std::vector<SafariBattleMenuOption>>> SafariOptimalAction::get_optimal_actions(
    ConsoleHandle& console,
    const std::string& pokemon_name,
    int balls_remaining
) const{
    if (balls_remaining < 1 || balls_remaining > 30){
        console.log("Invalid number of Safari Balls remaining: " + std::to_string(balls_remaining));
        return std::nullopt;
    }

    auto iter = m_action_table_by_pokemon.find(pokemon_name);
    if (iter == m_action_table_by_pokemon.end()){
        console.log("Unknown Pokemon: " + pokemon_name);
        return std::nullopt;
    }

    const std::vector<std::vector<SafariBattleMenuOption>>& action_table = iter->second;
    if ((size_t)balls_remaining > action_table.size()){
        console.log("Invalid number of Safari Balls remaining: " + std::to_string(balls_remaining));
        return std::nullopt;
    }

    const std::vector<SafariBattleMenuOption>& actions = action_table[balls_remaining - 1];
    if (actions.empty()){
        console.log("No actions available for Pokemon: " + pokemon_name + " with " + std::to_string(balls_remaining) + " Safari Balls remaining.");
        return std::nullopt;
    }

    return std::cref(actions);
}

}
}
}
