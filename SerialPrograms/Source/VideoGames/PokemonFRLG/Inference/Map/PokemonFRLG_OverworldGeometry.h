/*  Overworld Geometry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Where things are on the overworld screen, in GBA pixels and tiles.
 *
 *  The GBA screen is 240x160 pixels, made of 16x16 tiles. The camera keeps
 *  the player on the same screen tile: the one whose top-left corner is at
 *  (112, 72). A TileOffset is a tile's position relative to the player's
 *  (right and down positive).
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_OverworldGeometry_H
#define PokemonAutomation_PokemonFRLG_OverworldGeometry_H

#include <stddef.h>
#include <stdint.h>
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


constexpr int GBA_SCREEN_WIDTH = 240;
constexpr int GBA_SCREEN_HEIGHT = 160;
constexpr int OVERWORLD_TILE_SIZE = 16;
constexpr int PLAYER_TILE_LEFT = 112;
constexpr int PLAYER_TILE_TOP = 72;


struct TileOffset{
    int dx = 0;
    int dy = 0;
    bool operator==(const TileOffset& x) const{
        return dx == x.dx && dy == x.dy;
    }
};


//  The pixel at the center of GBA pixel (x, y) of `game_screen`, which is the
//  game's area of the video frame (see GameSettings::GAME_BOX) at any size.
inline uint32_t gba_pixel(const ImageViewRGB32& game_screen, int x, int y){
    size_t px = (size_t)((x + 0.5) * (double)game_screen.width() / GBA_SCREEN_WIDTH);
    size_t py = (size_t)((y + 0.5) * (double)game_screen.height() / GBA_SCREEN_HEIGHT);
    return game_screen.pixel(px, py);
}



}
}
}
#endif
