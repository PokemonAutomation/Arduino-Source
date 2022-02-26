/*  Pokemon LA Pokemon Sprites
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "PokemonLA_PokemonSprites.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{




struct PokemonSpriteDatabase{
    QImage m_sprites;
    QImage m_silhouettes;
    std::map<std::string, PokemonSprite> m_slug_to_data;

    static PokemonSpriteDatabase& instance(){
        static PokemonSpriteDatabase data;
        return data;
    }
    PokemonSpriteDatabase()
        : m_sprites(RESOURCE_PATH() + "PokemonLA/PokemonSprites.png")
    {
        QJsonObject json = read_json_file(
            RESOURCE_PATH() + "PokemonLA/PokemonSprites.json"
        ).object();

        int width = json.find("spriteWidth")->toInt();
        int height = json.find("spriteHeight")->toInt();
        if (width <= 0){
            PA_THROW_ParseException("Invalid width.");
        }
        if (height <= 0){
            PA_THROW_ParseException("Invalid height.");
        }

        QJsonObject locations = json.find("spriteLocations")->toObject();
        for (auto iter = locations.begin(); iter != locations.end(); ++iter){
            std::string slug = iter.key().toStdString();
            QJsonObject obj = iter.value().toObject();
            int y = obj.find("top")->toInt();
            int x = obj.find("left")->toInt();

            PokemonSprite sprite;
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


const std::map<std::string, PokemonSprite>& all_pokemon_sprites(){
    return PokemonSpriteDatabase::instance().m_slug_to_data;
}
const PokemonSprite& get_pokemon_sprite(const std::string& slug){
    const std::map<std::string, PokemonSprite>& database = PokemonSpriteDatabase::instance().m_slug_to_data;
    auto iter = database.find(slug);
    if (iter == database.end()){
        PA_THROW_StringException("Pokemon slug not found in database: " + slug);
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
