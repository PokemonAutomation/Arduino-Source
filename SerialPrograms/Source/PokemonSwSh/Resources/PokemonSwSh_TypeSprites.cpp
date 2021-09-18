/*  Pokemon Sword/Shield Type Sprites
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "Common/Cpp/Exception.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/ImageTools/CommonFilters.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "PokemonSwSh_TypeSprites.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{




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
    QString path = PERSISTENT_SETTINGS().resource_path + "PokemonSwSh/Types/" + QString::fromStdString(slug) + ".png";
    m_sprite = QImage(path);
    if (m_sprite.isNull()){
        PA_THROW_FileException("Unable to open file.", path);
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
    CellMatrix matrix(m_sprite);
    WhiteFilter filter(224);
    matrix.apply_filter(m_sprite, filter);
    std::vector<FillGeometry> objects = find_all_objects(matrix, 1, true);

    FillGeometry object;
    for (const FillGeometry& item : objects){
//        cout << item.center_x() << "," << item.center_y() << endl;
        object.merge_assume_no_overlap(item);
    }
//    cout << object.center_x() << "," << object.center_y() << endl;
//    cout << "[" << object.box.min_x << "," << object.box.min_y << "][" << object.box.max_x << "," << object.box.max_y << "]" << endl;

    m_matcher.reset(
        new ImageMatch::ExactMatchMetadata(
            m_sprite.copy(
                object.box.min_x,
                object.box.min_y,
                object.box.width(),
                object.box.height()
            )
        )
    );
    m_matching_object = object;
}


const TypeSprite& get_type_sprite(PokemonType type){
    const TypeSpriteDatabase& database = TypeSpriteDatabase::instance();
    auto it = database.m_type_map.find(type);
    if (it == database.m_type_map.end()){
        PA_THROW_StringException("Invalid type enum.");
    }
    return it->second;
}
const std::map<PokemonType, TypeSprite>& all_type_sprites(){
    return TypeSpriteDatabase::instance().m_type_map;
}




}
}
}
