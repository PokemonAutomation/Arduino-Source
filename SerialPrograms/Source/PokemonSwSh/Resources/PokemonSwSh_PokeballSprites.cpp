/*  Pokemon Sword/Shield Pokeball Sprites
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "PokemonSwSh_PokeballSprites.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



struct PokeballSpriteDatabase{
    QString m_path;
    QImage m_sprites;
    std::map<std::string, PokeballSprite> m_slug_to_data;

    static PokeballSpriteDatabase& instance(){
        static PokeballSpriteDatabase data;
        return data;
    }
    PokeballSpriteDatabase()
        : m_path(RESOURCE_PATH() + "PokemonSwSh/PokeballSprites.png")
        , m_sprites(m_path)
    {
        QJsonObject json = read_json_file(
            RESOURCE_PATH() + "PokemonSwSh/PokeballSprites.json"
        ).object();

        int width = json.find("spriteWidth")->toInt();
        int height = json.find("spriteHeight")->toInt();
        if (width <= 0){
            throw FileException(nullptr, __PRETTY_FUNCTION__, "Invalid width.", m_path.toStdString());
        }
        if (height <= 0){
            throw FileException(nullptr, __PRETTY_FUNCTION__, "Invalid height.", m_path.toStdString());
        }

        QJsonObject locations = json.find("spriteLocations")->toObject();
        for (auto iter = locations.begin(); iter != locations.end(); ++iter){
            std::string slug = iter.key().toStdString();
            QJsonObject obj = iter.value().toObject();
            int y = obj.find("top")->toInt();
            int x = obj.find("left")->toInt();

            PokeballSprite sprite;
            sprite.m_sprite = m_sprites.copy(x, y, width, height);

            if (sprite.m_sprite.format() != QImage::Format_ARGB32){
                sprite.m_sprite = sprite.m_sprite.convertToFormat(QImage::Format_ARGB32);
            }

            sprite.m_icon = QIcon(QPixmap::fromImage(ImageMatch::trim_image_alpha(sprite.m_sprite)));

            m_slug_to_data.emplace(
                slug,
                std::move(sprite)
            );
        }
    }
};


const std::map<std::string, PokeballSprite>& all_pokeball_sprites(){
    return PokeballSpriteDatabase::instance().m_slug_to_data;
}

const PokeballSprite& get_pokeball_sprite(const std::string& slug){
    const std::map<std::string, PokeballSprite>& database = PokeballSpriteDatabase::instance().m_slug_to_data;
    auto iter = database.find(slug);
    if (iter == database.end()){
        PA_THROW_StringException("Pokeball slug not found in database: " + slug);
    }
    return iter->second;
}
const PokeballSprite* get_pokeball_sprite_nothrow(const std::string& slug){
    const std::map<std::string, PokeballSprite>& database = PokeballSpriteDatabase::instance().m_slug_to_data;
    auto iter = database.find(slug);
    if (iter == database.end()){
        return nullptr;
    }
    return &iter->second;
}



}
}
}
