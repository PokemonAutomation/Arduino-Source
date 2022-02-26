/*  Pokemon LA Pokemon Sprites
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_PokemonSprites_H
#define PokemonAutomation_PokemonLA_PokemonSprites_H

#include <QIcon>
#include <QImage>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class PokemonSprite{
public:
    const QIcon& icon() const{ return m_icon; }
    const QImage& sprite() const{ return m_sprite; }

private:
    friend struct PokemonSpriteDatabase;

    QIcon m_icon;
    QImage m_sprite;
};

const std::map<std::string, PokemonSprite>& all_pokemon_sprites();

const PokemonSprite& get_pokemon_sprite(const std::string& slug);
const PokemonSprite* get_pokemon_sprite_nothrow(const std::string& slug);



}
}
}
#endif
