/*  Pokemon Name Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/PersistentSettings.h"
#include "Pokemon_NameSelect.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Pokemon{


PokemonNameSelectData::PokemonNameSelectData(const QString& json_file){
    QJsonArray array = read_json_file(
        PERSISTENT_SETTINGS().resource_path + json_file
    ).array();
    QJsonObject obj = read_json_file(
        PERSISTENT_SETTINGS().resource_path + "Pokemon/PokemonNameOCR/PokemonOCR-eng.json"
    ).object();
    for (const auto& item : array){
        QString token = item.toString();
//        cout << token.toUtf8().data() << endl;
        if (token.size() <= 0){
            PA_THROW_StringException("Expected non-empty string for Pokemon token.");
        }
        auto iter = obj.find(token);
        if (iter == obj.end()){
            PA_THROW_StringException("Pokemon token not found in database: " + token);
        }
        QJsonArray array = iter.value().toArray();
        if (array.empty()){
            PA_THROW_StringException("No display names or candidates found for: " + token);
        }
        QString display = array[0].toString();
        if (display.size() <= 0){
            PA_THROW_StringException("Expected non-empty string for display name. Token: " + token);
        }
        m_list.emplace_back(display);
        m_display_to_token.emplace(
            std::move(display),
            token.toUtf8().data()
        );
    }
}



PokemonNameSelect::PokemonNameSelect(
    QString label,
    const QString& json_file
)
    : PokemonNameSelectData(json_file)
    , StringSelect(std::move(label), cases(), 0)
{}

const std::string& PokemonNameSelect::token() const{
    const QString& display = (const QString&)*this;
    auto iter = m_display_to_token.find(display);
    if (iter == m_display_to_token.end()){
        PA_THROW_StringException("Display name not found in database: " + display);
    }
    return iter->second;
}




}
}
