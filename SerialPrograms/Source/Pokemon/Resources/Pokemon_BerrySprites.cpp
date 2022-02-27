/*  Pokemon Berry Sprites
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/ImageOpener.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "Pokemon_BerrySprites.h"

namespace PokemonAutomation{
namespace Pokemon{




struct BerrySpriteDatabase{
    QString m_path;
    QImage m_sprites;
    std::map<std::string, BerrySprite> m_slug_to_data;

    static BerrySpriteDatabase& instance(){
        static BerrySpriteDatabase data;
        return data;
    }
    BerrySpriteDatabase()
        : m_path(RESOURCE_PATH() + "Pokemon/BerrySprites.png")
        , m_sprites(open_image(m_path))
    {
        const QJsonObject json = read_json_file(RESOURCE_PATH() + "Pokemon/BerrySprites.json").object();

        const QJsonObject locations = json.find("spriteLocations")->toObject();
        for (auto iter = locations.begin(); iter != locations.end(); ++iter){
            const std::string slug = iter.key().toStdString();
            const QJsonObject obj = iter.value().toObject();
            const int y = obj.find("top")->toInt();
            const int x = obj.find("left")->toInt();
            const int width = obj.find("width")->toInt();
            const int height = obj.find("height")->toInt();
            if (width <= 0){
                throw FileException(nullptr, __PRETTY_FUNCTION__, "Invalid width.", m_path.toStdString());
            }
            if (height <= 0){
                throw FileException(nullptr, __PRETTY_FUNCTION__, "Invalid height.", m_path.toStdString());
            }

            BerrySprite sprite;
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


const std::map<std::string, BerrySprite>& all_berry_sprites(){
    return BerrySpriteDatabase::instance().m_slug_to_data;
}
const BerrySprite& get_berry_sprite(const std::string& slug){
    const std::map<std::string, BerrySprite>& database = BerrySpriteDatabase::instance().m_slug_to_data;
    auto iter = database.find(slug);
    if (iter == database.end()){
        PA_THROW_StringException("Berry slug not found in berry sprite database: " + slug);
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
