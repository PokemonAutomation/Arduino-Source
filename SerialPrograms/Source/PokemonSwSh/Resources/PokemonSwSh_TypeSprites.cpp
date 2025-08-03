/*  Pokemon Sword/Shield Type Sprites
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "PokemonSwSh_TypeSprites.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Kernels;
using namespace Kernels::Waterfill;




struct TypeSpriteDatabase{
    std::map<PokemonType, TypeSprite> m_type_map;

    static TypeSpriteDatabase& instance(){
        static TypeSpriteDatabase data;
        return data;
    }

    TypeSpriteDatabase(){
        for (const auto& item : POKEMON_TYPE_SLUGS()){
            if (item.first == PokemonType::NONE){
                continue;
            }
            m_type_map.emplace(item.first, item.second);
        }
    }

};






TypeSprite::TypeSprite(const std::string& slug)
    : m_slug(slug)
{
    ImageRGB32 sprite(RESOURCE_PATH() + "PokemonSwSh/Types/" + slug + ".png");

    //  Set all non-255 alphas to zero.
    size_t words = sprite.bytes_per_row() / sizeof(uint32_t);
    uint32_t* ptr = sprite.data();
    for (size_t r = 0; r < sprite.height(); r++){
        for (size_t c = 0; c < sprite.width(); c++){
//            cout << qAlpha(sprite.pixel(c, r)) << " ";
            uint32_t pixel = ptr[c];
            if ((pixel >> 24) != 0xff){
                ptr[c] = 0;
            }
        }
//        cout << endl;
        ptr += words;
    }

    //  Compute white objects.
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_min(sprite, 224, 224, 224);

    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 10);

    WaterfillObject object;
    for (const WaterfillObject& item : objects){
//        cout << item.center_x() << "," << item.center_y() << endl;
        object.merge_assume_no_overlap(item);
    }

    m_aspect_ratio = object.aspect_ratio();

//    sprite.save("symbol-" + std::to_string(slug) + ".png");
    m_matcher.reset(
        new ImageMatch::WeightedExactImageMatcher(
            sprite.sub_image(object.min_x, object.min_y, object.width(), object.height()).copy(),
            ImageMatch::WeightedExactImageMatcher::InverseStddevWeight{1, 64}
        )
    );
}


const TypeSprite& get_type_sprite(PokemonType type){
    const TypeSpriteDatabase& database = TypeSpriteDatabase::instance();
    auto it = database.m_type_map.find(type);
    if (it == database.m_type_map.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid type enum.");
    }
    return it->second;
}
const std::map<PokemonType, TypeSprite>& all_type_sprites(){
    return TypeSpriteDatabase::instance().m_type_map;
}




}
}
}
