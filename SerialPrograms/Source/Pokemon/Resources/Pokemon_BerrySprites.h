/*  Pokemon Berry Sprites
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_BerrySprites_H
#define PokemonAutomation_Pokemon_BerrySprites_H

#include <QIcon>
#include <QImage>

namespace PokemonAutomation{
namespace Pokemon{


class BerrySprite{
public:
    const QIcon& icon() const{ return m_icon; }
    const QImage& sprite() const{ return m_sprite; }

private:
    friend struct BerrySpriteDatabase;

    QIcon m_icon;
    QImage m_sprite;
};

const std::map<std::string, BerrySprite>& all_berry_sprites();

const BerrySprite& get_berry_sprite(const std::string& slug);
const BerrySprite* get_berry_sprite_nothrow(const std::string& slug);



}
}
#endif
