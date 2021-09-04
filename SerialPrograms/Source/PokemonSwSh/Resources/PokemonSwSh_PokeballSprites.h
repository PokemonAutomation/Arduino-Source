/*  Pokemon Sword/Shield Pokeball Sprites
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_PokeballSprites_H
#define PokemonAutomation_PokemonSwSh_PokeballSprites_H

#include <QIcon>
#include <QImage>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class PokeballSprite{
public:
    const QIcon& icon() const{ return m_icon; }
    const QImage& sprite() const{ return m_sprite; }

private:
    friend struct PokeballSpriteDatabase;

    QIcon m_icon;
    QImage m_sprite;
};

const std::map<std::string, PokeballSprite>& all_pokeball_sprites();

const PokeballSprite& get_pokeball_sprite(const std::string& slug);
const PokeballSprite* get_pokeball_sprite_nothrow(const std::string& slug);



}
}
}
#endif
