/*  Pokemon Icons
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/ImageOpener.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "CommonFramework/Globals.h"
#include "Pokemon_PokemonIcons.h"

#include <QImage>
#include <QPixmap>


namespace PokemonAutomation{
namespace Pokemon{


std::map<std::string, QIcon> load_pokemon_icons(const char* sprite_path, const char* json_path){
    QString path = RESOURCE_PATH() + sprite_path;
    QImage sprites = open_image(path);
    QJsonObject json = read_json_file(
        RESOURCE_PATH() + json_path
    ).object();

    int width = json.find("spriteWidth")->toInt();
    int height = json.find("spriteHeight")->toInt();
    if (width <= 0){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Invalid width.", path.toStdString());
    }
    if (height <= 0){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Invalid height.", path.toStdString());
    }

    std::map<std::string, QIcon> map;
    QJsonObject locations = json.find("spriteLocations")->toObject();
    for (auto iter = locations.begin(); iter != locations.end(); ++iter){
        std::string slug = iter.key().toStdString();
        QJsonObject obj = iter.value().toObject();
        int y = obj.find("top")->toInt();
        int x = obj.find("left")->toInt();

        QImage sprite = sprites.copy(x, y, width, height);
        QPixmap pixmap = QPixmap::fromImage(ImageMatch::trim_image_alpha(sprite));
        map.emplace(std::move(slug), pixmap);
    }
    return map;
}

}
}
