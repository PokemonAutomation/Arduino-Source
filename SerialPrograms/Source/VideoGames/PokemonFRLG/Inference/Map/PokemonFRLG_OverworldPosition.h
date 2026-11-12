/*  Overworld Position
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Tell which map tile the player is on, or whether a step happened, from
 *  what the screen looks like. These all rely on the camera keeping the
 *  player on the same screen tile (see PokemonFRLG_OverworldGeometry.h).
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_OverworldPosition_H
#define PokemonAutomation_PokemonFRLG_OverworldPosition_H

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <vector>

namespace PokemonAutomation{
    class ImageViewRGB32;
namespace NintendoSwitch{
namespace PokemonFRLG{


//  Remembers screens seen at known tiles, and finds the player's tile by
//  matching the current screen against them. The screen seen from one tile
//  is the screen seen from a neighboring tile shifted by 16 pixels, so one
//  remembered screen covers the tiles around it.
//
//  Screens are remembered at tiles whose position is certain (e.g. just after
//  a door or a bump into a wall). Works wherever the scenery has texture;
//  water is left out of the comparison since it animates. Coordinates are
//  whatever the caller uses for the map.
class ScreenOdometry{
public:
    void clear();
    size_t size() const{
        return m_refs.size();
    }

    //  The screen as seen from tile (x, y), with the player standing still.
    //  Replaces a screen remembered at the same tile; keeps the newest 40.
    void remember(const ImageViewRGB32& frame, int x, int y);

    //  Whether a screen was remembered within `dx` tiles across and `dy` tiles
    //  up or down of (x, y).
    bool has_reference_within(int x, int y, int dx, int dy) const;

    //  Find the player's tile, assuming it is within `radius` tiles across
    //  (and `radius_y` up or down, or `radius` if negative) of the guess.
    //  Returns false if no remembered screen overlaps enough, or the best
    //  match isn't clearly better than the next (e.g. mid-step). `details`
    //  describes the match, for logging.
    bool locate(
        const ImageViewRGB32& frame,
        int guess_x, int guess_y, int radius,
        int& x, int& y, std::string& details,
        int radius_y = -1
    ) const;

private:
    struct Reference{
        int x;
        int y;
        std::vector<uint8_t> gray;  //  240 x 160
    };
    std::vector<Reference> m_refs;
};


//  Indoors, everything outside the map is black, so the black margins tell
//  which tile the player is on. `map_width` is the map's width in tiles;
//  `floor_rows` is how many rows from the top are not black (rows below them
//  are drawn black like the outside, as the bottom row of most houses is).
//  Only for maps narrower than the screen (fewer than 15 tiles), where a left
//  or right margin is always visible. Not found mid-step or during a fade.
struct IndoorTile{
    bool found = false;
    int x = 0;
    int y = 0;
};
IndoorTile read_indoor_position(const ImageViewRGB32& frame, int map_width, int floor_rows);


//  The roof of the Chansey house in Sevault Canyon (its purple part), in GBA
//  pixels: a landmark for the player's tile in the canyon.
struct HouseRoof{
    bool found = false;
    int left = 0;       //  The roof's left edge.
    int top = -1;       //  First row of the purple part, or -1 if it may be cut off.
    int bottom = -1;    //  Row after its last row, or -1 if it may be cut off.
};
HouseRoof find_house_roof(const ImageViewRGB32& frame);


//  Compare frames taken before and after trying to take one step in the
//  direction (dx, dy) (one of them is 0, the other +1 or -1). Returns true if
//  the background moved by one tile, i.e. the step happened.
bool screen_scrolled_one_tile(
    const ImageViewRGB32& before, const ImageViewRGB32& after,
    int dx, int dy
);



}
}
}
#endif
