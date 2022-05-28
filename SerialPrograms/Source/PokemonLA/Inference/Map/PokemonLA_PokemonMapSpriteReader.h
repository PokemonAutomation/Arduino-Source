/*  Pokemon Map Sprite Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#ifndef PokemonAutomation_PokemonLA_PokemonMapSpriteReader_H
#define PokemonAutomation_PokemonLA_PokemonMapSpriteReader_H

class QImage;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


std::string match_pokemon_map_sprite(const QImage& image);


}
}
}
#endif
