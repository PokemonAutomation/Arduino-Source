/*  Pokemon Sword/Shield Type Sprites
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "PokemonSwSh_TypeSprites.h"

#include <iostream>
using std::cout;
using std::endl;

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
        for (const auto& item : TYPE_ENUM_TO_SLUG){
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
    QString path = RESOURCE_PATH() + "PokemonSwSh/Types/" + QString::fromStdString(slug) + ".png";
    m_sprite = QImage(path);
    if (m_sprite.isNull()){
        throw FileException(
            nullptr, PA_CURRENT_FUNCTION,
            "Unable to open file.",
            path.toStdString()
        );
    }
    if (m_sprite.format() != QImage::Format_RGB32 && m_sprite.format() != QImage::Format_ARGB32){
        m_sprite = m_sprite.convertToFormat(QImage::Format_RGB32);
    }

    //  Set all non-255 alphas to zero.
    size_t words = m_sprite.bytesPerLine() / sizeof(uint32_t);
    uint32_t* ptr = (uint32_t*)m_sprite.bits();
    for (int r = 0; r < m_sprite.height(); r++){
        for (int c = 0; c < m_sprite.width(); c++){
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
    PackedBinaryMatrix2 matrix = compress_rgb32_to_binary_min(m_sprite, 224, 224, 224);

    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 10);

    WaterfillObject object;
    for (const WaterfillObject& item : objects){
//        cout << item.center_x() << "," << item.center_y() << endl;
        object.merge_assume_no_overlap(item);
    }

    QImage sprite = m_sprite.copy(
        (pxint_t)object.min_x,
        (pxint_t)object.min_y,
        (pxint_t)object.width(),
        (pxint_t)object.height()
    );
//    sprite.save("symbol-" + QString::fromStdString(slug) + ".png");
    m_matcher.reset(
        new ImageMatch::WeightedExactImageMatcher(
            std::move(sprite),
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
