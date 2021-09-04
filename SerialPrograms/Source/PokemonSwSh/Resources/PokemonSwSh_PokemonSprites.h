/*  Pokemon Sword/Shield Pokemon Sprites
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_PokemonSprites_H
#define PokemonAutomation_PokemonSwSh_PokemonSprites_H

#include <QIcon>
#include <QImage>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class PokemonSprite{
public:
    const QIcon& icon() const{ return m_icon; }
    const QImage& sprite() const{ return m_sprite; }
    const QImage& silhouette() const{ return m_silhouette; }

private:
    friend struct PokemonSpriteDatabase;

    QIcon m_icon;
    QImage m_sprite;
    QImage m_silhouette;
};

const std::map<std::string, PokemonSprite>& all_pokemon_sprites();

const PokemonSprite& get_pokemon_sprite(const std::string& slug);
const PokemonSprite* get_pokemon_sprite_nothrow(const std::string& slug);



}
}
}
#endif
