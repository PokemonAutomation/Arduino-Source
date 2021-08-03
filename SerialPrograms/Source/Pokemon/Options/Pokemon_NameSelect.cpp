/*  Pokemon Name Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/PersistentSettings.h"
#include "Pokemon/Pokemon_SpeciesDatabase.h"
#include "Pokemon_NameSelect.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Pokemon{


PokemonNameSelectData::PokemonNameSelectData(const QString& json_file_slugs){
    QJsonArray array = read_json_file(
        PERSISTENT_SETTINGS().resource_path + json_file_slugs
    ).array();
    for (const auto& item : array){
        QString slug = item.toString();
        std::string slug_str = slug.toUtf8().data();
        if (slug.size() <= 0){
            PA_THROW_StringException("Expected non-empty string for Pokemon slug.");
        }
        const SpeciesData& data = species_slug_to_data(slug_str);
        m_list.emplace_back(data.display_name());
    }
}



PokemonNameSelect::PokemonNameSelect(
    QString label,
    const QString& json_file_slugs
)
    : PokemonNameSelectData(json_file_slugs)
    , StringSelect(std::move(label), cases(), 0)
{}

const std::string& PokemonNameSelect::slug() const{
    const QString& display = (const QString&)*this;
    return species_display_name_to_slug(display);
}




}
}
