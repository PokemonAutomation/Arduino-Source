/*  Overworld Sprites
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_OverworldSprites.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


namespace{

//  The purple hair of the Cooltrainer sprites, about (108, 83, 187). (The
//  blue windows of houses have less red than green; the hair has more.)
bool is_hair(uint32_t pixel){
    int r = (pixel >> 16) & 0xff;
    int g = (pixel >> 8) & 0xff;
    int b = pixel & 0xff;
    return b > 140 && b - r > 45 && b - g > 65 && r >= g;
}

//  Sprite outlines are near black.
bool is_outline(uint32_t pixel){
    int r = (pixel >> 16) & 0xff;
    int g = (pixel >> 8) & 0xff;
    int b = pixel & 0xff;
    return r + g + b < 120;
}

}


ImageFloatBox head_box_for_tile(const TileOffset& tile){
    //  A sprite's head sits on the tile above the one it stands on. The hair
    //  ends about 1 pixel above the tile's top edge. The box reaches higher to
    //  cover a sprite that is mid-hop.
    double left = PLAYER_TILE_LEFT + OVERWORLD_TILE_SIZE * tile.dx + 2;
    double top  = PLAYER_TILE_TOP + OVERWORLD_TILE_SIZE * tile.dy - 13;
    return ImageFloatBox(
        left / GBA_SCREEN_WIDTH, top / GBA_SCREEN_HEIGHT,
        12.0 / GBA_SCREEN_WIDTH, 18.0 / GBA_SCREEN_HEIGHT
    );
}

bool head_box_on_screen(const TileOffset& tile){
    ImageFloatBox box = head_box_for_tile(tile);
    return box.x >= 0 && box.y >= 0 && box.x + box.width <= 1 && box.y + box.height <= 1;
}


SpriteSighting find_purple_hair(const ImageViewRGB32& frame, const std::vector<TileOffset>& tiles){
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    SpriteSighting best;
    if (game_screen.width() == 0 || game_screen.height() == 0){
        return best;
    }
    const double scale_y = (double)game_screen.height() / GBA_SCREEN_HEIGHT;

    for (const TileOffset& tile : tiles){
        if (!head_box_on_screen(tile)){
            continue;
        }
        ImageFloatBox float_box = head_box_for_tile(tile);
        ImageViewRGB32 box = extract_box_reference(game_screen, float_box);
        size_t total = box.width() * box.height();
        if (total == 0){
            continue;
        }
        size_t count = 0;
        double sum_y = 0;
        for (size_t y = 0; y < box.height(); y++){
            for (size_t x = 0; x < box.width(); x++){
                if (is_hair(box.pixel(x, y))){
                    count++;
                    sum_y += (double)y;
                }
            }
        }
        double fraction = (double)count / (double)total;
        if (fraction > best.score){
            best.tile = tile;
            best.score = fraction;
            best.y = float_box.y * GBA_SCREEN_HEIGHT + (sum_y / (double)count) / scale_y;
        }
    }

    //  The hair covers about a third of the box; a few stray pixels don't count.
    best.found = best.score > 0.08;
    return best;
}


SpriteSighting find_sprite_feet(const ImageViewRGB32& frame, const std::vector<TileOffset>& tiles){
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    SpriteSighting best;
    if (game_screen.width() == 0 || game_screen.height() == 0){
        return best;
    }

    for (const TileOffset& tile : tiles){
        //  The tile's columns (without the 1-pixel edges), from 8 pixels
        //  above the tile (a standing sprite's legs) to its bottom edge.
        int left = PLAYER_TILE_LEFT + OVERWORLD_TILE_SIZE * tile.dx + 1;
        int right = left + OVERWORLD_TILE_SIZE - 2;
        int bottom = PLAYER_TILE_TOP + OVERWORLD_TILE_SIZE * tile.dy + OVERWORLD_TILE_SIZE;
        int top = bottom - OVERWORLD_TILE_SIZE - 8;
        top = std::max(top, 0);
        bottom = std::min(bottom, GBA_SCREEN_HEIGHT);
        if (left < 0 || right > GBA_SCREEN_WIDTH || bottom <= top){
            continue;
        }

        size_t count = 0;
        int lowest = -1;
        for (int y = top; y < bottom; y++){
            size_t row = 0;
            for (int x = left; x < right; x++){
                if (is_outline(gba_pixel(game_screen, x, y))){
                    row++;
                }
            }
            count += row;
            if (row >= 2){
                lowest = y;
            }
        }
        if (lowest >= 0 && (double)count > best.score){
            best.tile = tile;
            best.score = (double)count;
            best.y = lowest;
        }
    }

    //  The legs and shoes are at least a few rows of outline.
    best.found = best.score >= 6;
    return best;
}



}
}
}
