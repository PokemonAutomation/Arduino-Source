/*  Pokemon FRLG Pokemon Sprites
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_PokemonSprites_H
#define PokemonAutomation_PokemonFRLG_PokemonSprites_H

#include <set>
#include <string>
#include "CommonTools/Resources/SpriteDatabase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


// Front sprites, keyed by sprite slug. All four databases share the same slugs.
// Unown and Deoxys have multiple slugs for their different forms
const SpriteDatabase& ALL_POKEMON_SPRITES();
const SpriteDatabase& ALL_POKEMON_SHINY_SPRITES();

// Mirrored left-to-right, except for a few species
const SpriteDatabase& FLIPPED_POKEMON_SPRITES();
const SpriteDatabase& FLIPPED_POKEMON_SHINY_SPRITES();

const SpriteDatabase& POKEMON_SPRITES(bool shiny, bool flipped);


// Convert a sprite slug to the species slug by stripping form suffixes
const std::string& sprite_slug_to_species(const std::string& sprite_slug);

// Return true if the sprite should be included when searching for any of
// the species in "subset".
bool sprite_in_species_subset(const std::string& sprite_slug, const std::set<std::string>& subset);

// Return true if the game mirrors this sprite on the summary screen
bool sprite_flipped_on_summary(const std::string& sprite_slug);


}
}
}
#endif
