/*  Pokemon Icons
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include <QPixmap>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Qt/ImageOpener.h"
#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "CommonFramework/Globals.h"
#include "Pokemon_PokemonIcons.h"

namespace PokemonAutomation{
namespace Pokemon{


std::map<std::string, QIcon> load_pokemon_icons(const char* sprite_path, const char* json_path){
    QImage sprites = open_image(RESOURCE_PATH() + sprite_path);
    std::string path = RESOURCE_PATH().toStdString() + json_path;
    JsonValue json = load_json_file(path);
    JsonObject& root = json.get_object_throw(path);

    int width = root.get_integer_throw("spriteWidth", path);
    int height = root.get_integer_throw("spriteHeight", path);
    if (width <= 0){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Invalid width.", path);
    }
    if (height <= 0){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Invalid height.", path);
    }

    std::map<std::string, QIcon> map;
    JsonObject& locations = root.get_object_throw("spriteLocations", path);
    for (auto& item : locations){
        const std::string& slug = item.first;
        JsonObject& obj = item.second.get_object_throw(path);
        int y = (int)obj.get_integer_throw("top", path);
        int x = (int)obj.get_integer_throw("left", path);

        QImage sprite = sprites.copy(x, y, width, height);
        QPixmap pixmap = QPixmap::fromImage(ImageMatch::trim_image_alpha(sprite));
        map.emplace(slug, pixmap);
    }
    return map;
}

}
}
