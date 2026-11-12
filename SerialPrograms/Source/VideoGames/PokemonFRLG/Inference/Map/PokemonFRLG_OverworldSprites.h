/*  Overworld Sprites
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Find a person's sprite on given tiles around the player: by the purple
 *  hair of the Cooltrainer sprites, or by the black outline of any sprite.
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_OverworldSprites_H
#define PokemonAutomation_PokemonFRLG_OverworldSprites_H

#include <vector>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "PokemonFRLG_OverworldGeometry.h"

namespace PokemonAutomation{
    class ImageViewRGB32;
namespace NintendoSwitch{
namespace PokemonFRLG{


//  The box, as a fraction of the GBA screen, where a 16x32 sprite standing on
//  that tile draws the top of its head (with room for a hop).
ImageFloatBox head_box_for_tile(const TileOffset& tile);

//  Whether that box is entirely on screen. On the top two rows of the screen
//  a sprite's head is above the top edge.
bool head_box_on_screen(const TileOffset& tile);


struct SpriteSighting{
    bool found = false;
    TileOffset tile;
    double y = 0;           //  Where the sprite was seen, in GBA pixels. Rises during a hop.
    double score = 0;       //  How strongly it was seen (to pick the best tile).
};

//  Look for purple hair (the Cooltrainer sprites) in the head box of each of
//  the tiles and return the tile with the most. y is the hair's average
//  height. Not found if no tile has enough. Tiles whose head box is off
//  screen are skipped.
SpriteSighting find_purple_hair(const ImageViewRGB32& frame, const std::vector<TileOffset>& tiles);

//  Look for a sprite's black outline in the lower part of each tile (its legs
//  and feet) and return the tile with the most. y is the lowest outline row.
//  Works on any tile on screen, including the top rows where the head is cut
//  off. Not found if no tile has enough.
SpriteSighting find_sprite_feet(const ImageViewRGB32& frame, const std::vector<TileOffset>& tiles);



}
}
}
#endif
