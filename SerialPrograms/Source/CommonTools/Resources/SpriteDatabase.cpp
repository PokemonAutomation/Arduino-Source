/*  Sprite Composite Image
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/ImageMatch/ImageCropper.h"
#include "SpriteDatabase.h"

namespace PokemonAutomation{



SpriteDatabase::SpriteDatabase(const char* sprite_path, const char* json_path)
    : m_backing_image(RESOURCE_PATH() + sprite_path)
{
    std::string path = RESOURCE_PATH() + json_path;
    JsonValue json = load_json_file(path);
    JsonObject& root = json.to_object_throw(path);

    int64_t width = root.get_integer_throw("spriteWidth", path);
    int64_t height = root.get_integer_throw("spriteHeight", path);
    if (width <= 0){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Invalid width.", path);
    }
    if (height <= 0){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Invalid height.", path);
    }

    JsonObject& locations = root.get_object_throw("spriteLocations", path);
    for (auto& item : locations){
        const std::string& slug = item.first;
        JsonObject& obj = item.second.to_object_throw(path);
        int y = (int)obj.get_integer_throw("top", path);
        int x = (int)obj.get_integer_throw("left", path);

        ImageViewRGB32 sprite = extract_box_reference(m_backing_image, ImagePixelBox(x, y, x + width, y + height));
        m_database.emplace(
            slug,
            Sprite{sprite, ImageMatch::trim_image_alpha(sprite)}
        );
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
