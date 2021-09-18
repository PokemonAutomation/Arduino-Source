/*  Pokemon Sword/Shield Type Sprites
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_TypeSprites_H
#define PokemonAutomation_PokemonSwSh_TypeSprites_H

#include <memory>
#include "CommonFramework/ImageTools/FloatPixel.h"
#include "CommonFramework/ImageTools/FillGeometry.h"
#include "CommonFramework/ImageMatch/ImageMatchMetadata.h"
#include "PokemonSwSh_TypeMatchup.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class TypeSprite{
public:
    const std::string& slug() const{ return m_slug; }
    const QImage& sprite() const{ return m_sprite; }

    const ImageMatch::ExactMatchMetadata& matcher() const{ return *m_matcher; }
    const FillGeometry& matching_object() const{ return m_matching_object; }

public:
//    friend struct TypeSpriteDatabase;
    TypeSprite(const std::string& slug);

private:
    std::string m_slug;
    QImage m_sprite;

    std::unique_ptr<ImageMatch::ExactMatchMetadata> m_matcher;
    FillGeometry m_matching_object;
};


const TypeSprite& get_type_sprite(PokemonType type);
const std::map<PokemonType, TypeSprite>& all_type_sprites();




}
}
}
#endif
