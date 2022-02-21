/*  Available Pokemon
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
#include "PokemonLA_AvailablePokemon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


std::vector<std::string> load_hisui_dex(){
    QJsonArray json = read_json_file(
        RESOURCE_PATH() + "Pokemon/Pokedex/Pokedex-Hisui.json"
    ).array();

    std::vector<std::string> list;
    for (const auto& item : json){
        QString slug_qstr = item.toString();
        if (slug_qstr.size() <= 0){
            PA_THROW_StringException("Expected non-empty string for Pokemon slug.");
        }
        list.emplace_back(slug_qstr.toStdString());
    }
    return list;
}


const std::vector<std::string>& HISUI_DEX_SLUGS(){
    static const std::vector<std::string> database = load_hisui_dex();
    return database;
}



}
}
}
