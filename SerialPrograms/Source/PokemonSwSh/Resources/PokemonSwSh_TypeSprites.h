/*  Pokemon Sword/Shield Type Sprites
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_TypeSprites_H
#define PokemonAutomation_PokemonSwSh_TypeSprites_H

#include <memory>
#include <map>
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "Pokemon/Pokemon_Types.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;


class TypeSprite{
public:
    const std::string& slug() const{ return m_slug; }
    double aspect_ratio() const{ return m_aspect_ratio; }
//    const ImageRGB32& sprite() const{ return m_sprite; }

    const ImageMatch::WeightedExactImageMatcher& matcher() const{ return *m_matcher; }

public:
//    friend struct TypeSpriteDatabase;
    TypeSprite(const std::string& slug);

private:
    std::string m_slug;
    double m_aspect_ratio;

    std::unique_ptr<ImageMatch::WeightedExactImageMatcher> m_matcher;
};


const TypeSprite& get_type_sprite(PokemonType type);
const std::map<PokemonType, TypeSprite>& all_type_sprites();




}
}
}
#endif
