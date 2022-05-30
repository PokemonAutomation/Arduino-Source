/*  Pokemon Map Sprite Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#ifndef PokemonAutomation_PokemonLA_PokemonMapSpriteReader_H
#define PokemonAutomation_PokemonLA_PokemonMapSpriteReader_H


#include "CommonFramework/ImageMatch/ExactImageDictionaryMatcher.h"
#include "CommonFramework/ImageTypes/ImageReference.h"

#include <map>

class QImage;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


std::multimap<double, std::string> match_pokemon_map_sprite(const ConstImageRef& image);

const ImageMatch::ExactImageDictionaryMatcher& get_MMO_sprite_matcher();

}
}
}
#endif
