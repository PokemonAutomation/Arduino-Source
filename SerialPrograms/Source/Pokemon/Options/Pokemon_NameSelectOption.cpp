/*  Pokemon Name Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h"
#include "Pokemon_NameSelectOption.h"

namespace PokemonAutomation{
namespace Pokemon{



PokemonNameSelectData::PokemonNameSelectData(const std::vector<std::string>& slugs){
    for (const std::string& slug : slugs){
        if (slug.size() <= 0){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Expected non-empty string for Pokemon slug.");
        }

        using namespace NintendoSwitch::PokemonSwSh;
        const PokemonNames& data = get_pokemon_name(slug);
        const PokemonSprite* sprites = get_pokemon_sprite_nothrow(slug);
        if (sprites == nullptr){
            m_list.emplace_back(data.display_name(), QIcon());
            global_logger_tagged().log("Missing sprite for: " + slug, COLOR_RED);
        }else{
            m_list.emplace_back(
                data.display_name(),
                sprites->icon()
           );
        }
    }
}
PokemonNameSelectData::PokemonNameSelectData(const QString& json_file_slugs){
    std::string path = RESOURCE_PATH().toStdString() + json_file_slugs.toStdString();
    JsonValue json_slugs = load_json_file(path);
    JsonArray* slugs = json_slugs.get_array();
    if (slugs == nullptr){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Unable to load resource.", std::move(path));
    }

    for (auto& item : *slugs){
        std::string* slug = item.get_string();
        if (slug == nullptr || slug->empty()){
            throw FileException(
                nullptr, PA_CURRENT_FUNCTION,
                "Expected non-empty string for Pokemon slug.",
                std::move(path)
            );
        }

        using namespace NintendoSwitch::PokemonSwSh;
        const PokemonNames& data = get_pokemon_name(*slug);
        const PokemonSprite* sprites = get_pokemon_sprite_nothrow(*slug);
        if (sprites == nullptr){
            m_list.emplace_back(data.display_name(), QIcon());
            global_logger_tagged().log("Missing sprite for: " + *slug, COLOR_RED);
        }else{
            m_list.emplace_back(
                data.display_name(),
                sprites->icon()
           );
        }
    }
}



PokemonNameSelect::PokemonNameSelect(
    QString label,
    const std::vector<std::string>& slugs,
    const std::string& default_slug
)
    : PokemonNameSelectData(slugs)
    , StringSelectOption(
        std::move(label),
        cases(),
        default_slug.empty() ? "" :get_pokemon_name(default_slug).display_name()
    )
{}
PokemonNameSelect::PokemonNameSelect(
    QString label,
    const QString& json_file_slugs,
    const std::string& default_slug
)
    : PokemonNameSelectData(json_file_slugs)
    , StringSelectOption(
        std::move(label),
        cases(),
        default_slug.empty() ? "" :get_pokemon_name(default_slug).display_name()
    )
{}

const std::string& PokemonNameSelect::slug() const{
    const std::string& display = (const std::string&)*this;
    return parse_pokemon_name(display);
}




}
}
