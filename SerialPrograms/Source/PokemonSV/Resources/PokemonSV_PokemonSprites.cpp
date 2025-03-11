/*  Pokemon Scarlet/Violet Pokemon Sprites
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QImageReader>
#include "CommonFramework/Globals.h"
#include "CommonTools/ImageMatch/ImageCropper.h"
#include "PokemonSV_PokemonSprites.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

const SpriteDatabase& ALL_POKEMON_SPRITES(){
#if QT_VERSION_MAJOR == 6
    QImageReader::setAllocationLimit(0);
#endif
    static const SpriteDatabase database("PokemonSV/PokemonSprites.png", "PokemonSV/PokemonSprites.json");
    return database;
}

const SpriteDatabase& ALL_POKEMON_SILHOUETTES(){
#if QT_VERSION_MAJOR == 6
    QImageReader::setAllocationLimit(0);
#endif
    static const SpriteDatabase database("PokemonSV/PokemonSilhouettes.png", "PokemonSV/PokemonSprites.json");
    return database;
}

const std::array<std::string, NUM_TERA_TYPE> TERA_TYPE_NAMES = {
    "Bug",
    "Dark",
    "Dragon",
    "Electric",
    "Fairy",
    "Fighting",
    "Fire",
    "Flying",
    "Ghost",
    "Grass",
    "Ground",
    "Ice",
    "Normal",
    "Poison",
    "Psychic",
    "Rock",
    "Steel",
    "Water",
};
std::array<ImageRGB32, NUM_TERA_TYPE> BUILD_TERA_TYPE_ICONS(){
    const std::string image_folder_path = RESOURCE_PATH() + "PokemonSV/TeraTypes/";

    std::array<ImageRGB32, NUM_TERA_TYPE> ret;
    for (size_t i = 0; i < NUM_TERA_TYPE; i++){
        const auto& type_name = TERA_TYPE_NAMES[i];
        const std::string image_path = image_folder_path + type_name + ".png";

        // Trim the image to remove 0-alpha boundaries.
        ImageRGB32 image(ImageMatch::trim_image_alpha(ImageRGB32(image_path)).copy());
        ret[i] = std::move(image);
    }

    return ret;
}
const std::array<ImageRGB32, NUM_TERA_TYPE>& ALL_TERA_TYPE_ICONS(){
    const static auto icons = BUILD_TERA_TYPE_ICONS();
    return icons;
}

}
}
}
