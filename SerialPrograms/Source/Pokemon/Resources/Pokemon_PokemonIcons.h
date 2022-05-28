/*  Pokemon Icons
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_PokemonIcons_H
#define PokemonAutomation_Pokemon_PokemonIcons_H

#include <QIcon>

#include <string>
#include <map>

namespace PokemonAutomation{
namespace Pokemon{


// Load pokemon sprites from a single image file.
// The location of each sprite on the image is specified by a json file.
// Return a map of pokemon slug name for each sprite -> QIcon of the sprite.
//
// The json file must have the following format:
// { 
//   "spriteWidth": <width of each sprite>,
//   "spriteHeight": <height of each sprite>,
//   "spriteLocations": {
//     "<pokemon_slug_for sprite>" : {
//        "top": <how many pixels from this sprite to the top edge of the image>,
//        "left":  <how many pixels from the sprite to the left edge of the image>
//     },
//     (next pokemon) ...
//   }
// }
std::map<std::string, QIcon> load_pokemon_icons(const char* sprite_path, const char* json_path);


}
}
#endif
