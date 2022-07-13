/*  Sprite Composite Image
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Qt/ImageOpener.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "SpriteDatabase.h"

namespace PokemonAutomation{



SpriteDatabase::SpriteDatabase(const char* sprite_path, const char* json_path)
    : m_backing_image(open_image(RESOURCE_PATH() + sprite_path))
{
    std::string path = RESOURCE_PATH() + json_path;
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

    JsonObject& locations = root.get_object_throw("spriteLocations", path);
    for (auto& item : locations){
        const std::string& slug = item.first;
        JsonObject& obj = item.second.get_object_throw(path);
        int y = (int)obj.get_integer_throw("top", path);
        int x = (int)obj.get_integer_throw("left", path);

//        QImage sprite = sprites.copy(x, y, width, height);
        ConstImageRef sprite = extract_box_reference(m_backing_image, ImagePixelBox(x, y, x + width, y + height));
        QPixmap pixmap = QPixmap::fromImage(ImageMatch::trim_image_alpha(sprite));
        m_database.emplace(slug, Sprite{sprite, pixmap});
    }
}

const SpriteDatabase::Sprite& SpriteDatabase::get_throw(const std::string& slug) const{
    auto iter = m_database.find(slug);
    if (iter == m_database.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Slug not found in database: " + slug);
    }
    return iter->second;
}
const SpriteDatabase::Sprite* SpriteDatabase::get_nothrow(const std::string& slug) const{
    auto iter = m_database.find(slug);
    if (iter == m_database.end()){
        return nullptr;
    }
    return &iter->second;
}



}
