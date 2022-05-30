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

// Given an image of a pokemon sprite during MMO, match it against a loaded sprite database.
std::multimap<double, std::string> match_pokemon_map_sprite(const ConstImageRef& image);



const ImageMatch::ExactImageDictionaryMatcher& get_MMO_sprite_matcher();

const ImageMatch::ExactImageDictionaryMatcher& get_MMO_sprite_gradient_matcher();

QImage compute_MMO_sprite_gradient(const ConstImageRef& image);

float compute_MMO_sprite_gradient_distance(const ConstImageRef& gradient_template, const ConstImageRef& gradient);



}
}
}
#endif
