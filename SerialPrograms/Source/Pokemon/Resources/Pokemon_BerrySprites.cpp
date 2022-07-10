/*  Pokemon Berry Sprites
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Qt/ImageOpener.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "Pokemon_BerrySprites.h"

namespace PokemonAutomation{
namespace Pokemon{




struct BerrySpriteDatabase{
    QImage m_sprites;
    std::map<std::string, BerrySprite> m_slug_to_data;

    static BerrySpriteDatabase& instance(){
        static BerrySpriteDatabase data;
        return data;
    }
    BerrySpriteDatabase()
        : m_sprites(open_image(RESOURCE_PATH() + "Pokemon/BerrySprites.png"))
    {
        std::string path = RESOURCE_PATH().toStdString() + "Pokemon/BerrySprites.json";
        JsonValue json = load_json_file(path);
        JsonObject& root = json.get_object_throw(path);

        JsonObject& locations = root.get_object_throw("spriteLocations");
        for (auto& item : locations){
            const std::string& slug = item.first;
            JsonObject& obj = item.second.get_object_throw(path);
            const int y = obj.get_integer_throw("top", path);
            const int x = obj.get_integer_throw("left", path);
            const int width = obj.get_integer_throw("width", path);
            const int height = obj.get_integer_throw("height", path);
            if (width <= 0){
                throw FileException(nullptr, PA_CURRENT_FUNCTION, "Invalid width.", path);
            }
            if (height <= 0){
                throw FileException(nullptr, PA_CURRENT_FUNCTION, "Invalid height.", path);
            }

            BerrySprite sprite;
            sprite.m_sprite = m_sprites.copy(x, y, width, height);

            if (sprite.m_sprite.format() != QImage::Format_ARGB32){
                sprite.m_sprite = sprite.m_sprite.convertToFormat(QImage::Format_ARGB32);
            }

            sprite.m_icon = QIcon(QPixmap::fromImage(ImageMatch::trim_image_alpha(sprite.m_sprite)));

            m_slug_to_data.emplace(slug, std::move(sprite));
        }
    }
};


const std::map<std::string, BerrySprite>& all_berry_sprites(){
    return BerrySpriteDatabase::instance().m_slug_to_data;
}
const BerrySprite& get_berry_sprite(const std::string& slug){
    const std::map<std::string, BerrySprite>& database = BerrySpriteDatabase::instance().m_slug_to_data;
    auto iter = database.find(slug);
    if (iter == database.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Berry slug not found in berry sprite database: " + slug);
    }
    return iter->second;
}
const BerrySprite* get_berry_sprite_nothrow(const std::string& slug){
    const std::map<std::string, BerrySprite>& database = BerrySpriteDatabase::instance().m_slug_to_data;
    auto iter = database.find(slug);
    if (iter == database.end()){
        return nullptr;
    }
    return &iter->second;
}



}
}
