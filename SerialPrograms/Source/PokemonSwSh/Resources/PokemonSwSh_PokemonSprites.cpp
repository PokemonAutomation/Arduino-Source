/*  Pokemon Sword/Shield Pokemon Sprites
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "PokemonSwSh_PokemonSprites.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{




struct PokemonSpriteDatabase{
    QImage m_sprites;
    QImage m_silhouettes;
    std::map<std::string, PokemonSprite> m_slug_to_data;

    static PokemonSpriteDatabase& instance(){
        static PokemonSpriteDatabase data;
        return data;
    }
    PokemonSpriteDatabase()
        : m_sprites(RESOURCE_PATH() + "PokemonSwSh/PokemonSprites.png")
        , m_silhouettes(RESOURCE_PATH() + "PokemonSwSh/PokemonSilhouettes.png")
    {
        std::string path = RESOURCE_PATH().toStdString() + "PokemonSwSh/PokemonSprites.json";
        JsonValue json = load_json_file(path);
        JsonObject& root = json.get_object_throw(path);

        int width = (int)root.get_integer_throw("spriteWidth", path);
        int height = (int)root.get_integer_throw("spriteHeight", path);
        if (width <= 0){
            throw FileException(nullptr, PA_CURRENT_FUNCTION, "Invalid width.", path);
        }
        if (height <= 0){
            throw FileException(nullptr, PA_CURRENT_FUNCTION, "Invalid height.", path);
        }

        JsonObject& locations = root.get_object_throw("spriteLocations", path);
        for (auto& item : locations){
            const std::string& slug = item.first;
            JsonObject& obj = item.second.get_object_throw(path);
            int y = obj.get_integer_throw("top", path);
            int x = obj.get_integer_throw("left", path);

            PokemonSprite sprite;
            sprite.m_sprite = m_sprites.copy(x, y, width, height);
            sprite.m_silhouette = m_silhouettes.copy(x, y, width, height);

            if (sprite.m_sprite.format() != QImage::Format_ARGB32){
                sprite.m_sprite = sprite.m_sprite.convertToFormat(QImage::Format_ARGB32);
            }
            if (sprite.m_silhouette.format() != QImage::Format_ARGB32){
                sprite.m_silhouette = sprite.m_sprite.convertToFormat(QImage::Format_ARGB32);
            }

            sprite.m_icon = QIcon(QPixmap::fromImage(ImageMatch::trim_image_alpha(sprite.m_sprite)));
//            sprite.m_sprite_zoomed = trim_image(sprite.m_sprite_full);
//            sprite.m_silhouette_zoomed = trim_image(sprite.m_silhouette_full);

//            cout << width << " x " << height << endl;
//            cout << slug << ": " << sprite.m_sprite.width() << " x " << sprite.m_sprite.height() << endl;

            m_slug_to_data.emplace(slug, std::move(sprite));
        }
    }
};


const std::map<std::string, PokemonSprite>& all_pokemon_sprites(){
    return PokemonSpriteDatabase::instance().m_slug_to_data;
}
const PokemonSprite& get_pokemon_sprite(const std::string& slug){
    const std::map<std::string, PokemonSprite>& database = PokemonSpriteDatabase::instance().m_slug_to_data;
    auto iter = database.find(slug);
    if (iter == database.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Pokemon slug not found in database: " + slug);
    }
    return iter->second;
}
const PokemonSprite* get_pokemon_sprite_nothrow(const std::string& slug){
    const std::map<std::string, PokemonSprite>& database = PokemonSpriteDatabase::instance().m_slug_to_data;
    auto iter = database.find(slug);
    if (iter == database.end()){
        return nullptr;
    }
    return &iter->second;
}



}
}
}
