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


const SpriteDatabase& ALL_POKEMON_SPRITES(){
    static const SpriteDatabase database("PokemonSwSh/PokemonSprites.png", "PokemonSwSh/PokemonSprites.json");
    return database;
}
const SpriteDatabase& ALL_POKEMON_SILHOUETTES(){
    static const SpriteDatabase database("PokemonSwSh/PokemonSilhouettes.png", "PokemonSwSh/PokemonSprites.json");
    return database;
}



}
}
}
