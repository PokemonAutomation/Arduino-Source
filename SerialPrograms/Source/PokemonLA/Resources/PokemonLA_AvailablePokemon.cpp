/*  Available Pokemon
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
#include "PokemonLA_AvailablePokemon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


std::vector<std::string> load_pokemon_list_json(const char* json_path){
    QString path = RESOURCE_PATH() + json_path;
    QJsonArray json = read_json_file(path).array();

    std::vector<std::string> list;
    for (const auto& item : json){
        QString slug_qstr = item.toString();
        if (slug_qstr.size() <= 0){
            throw FileException(
                nullptr, PA_CURRENT_FUNCTION,
                "Expected non-empty string for Pokemon slug.",
                path.toStdString()
            );
        }
        list.emplace_back(slug_qstr.toStdString());
    }
    return list;
}


const std::vector<std::string>& HISUI_DEX_SLUGS(){
    static const std::vector<std::string> database = load_pokemon_list_json("Pokemon/Pokedex/Pokedex-Hisui.json");
    return database;
}

const std::vector<std::string>& HISUI_OUTBREAK_SLUGS(){
    static const std::vector<std::string> database = load_pokemon_list_json("PokemonLA/OutbreakList.json");
    return database;
}


}
}
}
