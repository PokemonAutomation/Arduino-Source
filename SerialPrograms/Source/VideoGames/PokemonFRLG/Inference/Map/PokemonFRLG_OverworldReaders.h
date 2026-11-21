/*  Overworld Readers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Overworld helpers that work in GBA tiles around the player:
 *    - find a trainer with purple hair (Cooltrainer sprites) on given tiles,
 *      or a sprite's feet on tiles at the top of the screen,
 *    - find the VS Seeker's "!!" / "X" icon above a trainer,
 *    - find the Sevault Canyon house's roof (a landmark for the player's tile),
 *    - tell whether the screen scrolled by one tile (did a step happen?),
 *    - detect any region's Fly map (Kanto or the Sevii Islands).
 *
 *  The camera keeps the player on the same tile of the screen: the 16x16
 *  tile whose top-left corner is at (112, 72) of the 240x160 GBA screen.
 *  (dx, dy) below are tile offsets from the player (right and down positive).
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_OverworldReaders_H
#define PokemonAutomation_PokemonFRLG_OverworldReaders_H

#include <chrono>
#include <string>
#include <vector>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
    class ImageViewRGB32;
    class VideoOverlaySet;
namespace NintendoSwitch{
namespace PokemonFRLG{


struct TileOffset{
    int dx = 0;
    int dy = 0;
    bool operator==(const TileOffset& x) const{ return dx == x.dx && dy == x.dy; }
};

//  The GBA-screen box where a 16x32 sprite standing on that tile draws the
//  top of its head.
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

//  Look for purple hair (the Cooltrainer sprites) on each of the tiles and
//  return the tile with the most. y is the hair's average height.
//  Not found if no tile has enough. Tiles whose head box is off screen are skipped.
SpriteSighting find_purple_hair(const ImageViewRGB32& frame, const std::vector<TileOffset>& tiles);

//  For tiles near the top of the screen, where the head is off screen: look
//  for a sprite's black outline in the visible part of the tile and return
//  the tile with the most. y is the lowest outline row (the feet).
SpriteSighting find_sprite_feet(const ImageViewRGB32& frame, const std::vector<TileOffset>& tiles);


//  Indoors, everything outside the map is black, and the camera keeps the
//  player on the same screen tile, so the black margins tell which map tile
//  the player is on. `map_width` is the map's width in tiles; `floor_rows` is
//  how many rows from the top are not black (rows below them are drawn black
//  like the outside, as the bottom row of most houses is).
//  Only for maps narrower than the screen (fewer than 15 tiles), where a
//  left or right margin is always visible.
struct IndoorTile{
    bool found = false;
    int x = 0;
    int y = 0;
};
IndoorTile read_indoor_position(const ImageViewRGB32& frame, int map_width, int floor_rows);


//  The icon a trainer shows right after the VS Seeker is used: "!!" (wants a
//  rematch) or "X" (doesn't). Each is a 16x16 speech bubble above the head,
//  shown for about half a second.
enum class VsSeekerIcon{
    NONE,
    READY,      //  "!!": white bubble, navy marks.
    NOT_READY,  //  "X": light blue bubble, white cross.
};
//  Look for either icon inside a region of the screen, in GBA pixels.
VsSeekerIcon find_vs_seeker_icon(const ImageViewRGB32& frame, int left, int top, int right, int bottom);

//  Every "!!" or "X" icon on the screen, with its center in GBA pixels. An
//  icon is drawn centered over the trainer's tile, its center 17 pixels
//  above the tile's top edge.
struct VsSeekerIconSighting{
    VsSeekerIcon icon = VsSeekerIcon::NONE;
    int x = 0;
    int y = 0;
};
std::vector<VsSeekerIconSighting> find_vs_seeker_icons(const ImageViewRGB32& frame);


//  Tells where the player is from what the screen looks like. The camera
//  keeps the player on the same screen tile, so the screen seen from one
//  tile is the screen seen from a neighboring tile shifted by 16 pixels.
//  Screens are remembered at tiles whose position is certain (e.g. just
//  after a door or map warp); later screens are matched against them at
//  each possible tile offset. Works wherever the scenery has texture (not
//  on open water). Coordinates are whatever the caller uses for the map.
class ScreenOdometry{
public:
    void clear();
    size_t size() const{ return m_refs.size(); }

    //  The screen as seen from tile (x, y), with the player standing still.
    void remember(const ImageViewRGB32& frame, int x, int y);

    //  Whether a screen was remembered within `distance` tiles of (x, y).
    bool has_reference_near(int x, int y, int distance) const;
    //  ... within `dx` tiles across and `dy` up or down.
    bool has_reference_within(int x, int y, int dx, int dy) const;

    //  Find the player's tile, assuming it is within `radius` tiles of the
    //  guess (`radius_y` up or down, if given). False if no remembered screen
    //  overlaps enough, or the best match isn't clearly better than the
    //  others (e.g. mid-step).
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


//  The roof of the Chansey house in Sevault Canyon (its purple part), in GBA pixels. Since the
//  camera keeps the player on the same screen tile, where the roof is on
//  screen tells which tile the player is on.
struct HouseRoof{
    bool found = false;
    int left = 0;       //  The roof's left edge.
    int top = -1;       //  First row of the purple part, or -1 if it may be cut off.
    int bottom = -1;    //  Row after its last row, or -1 if it may be cut off.
};
HouseRoof find_house_roof(const ImageViewRGB32& frame);


//  Compare two frames taken before and after trying to take one step in the
//  direction (dx, dy) (one of them is 0, the other +/-1). Returns true if the
//  background moved by one tile, i.e. the step happened.
bool screen_scrolled_one_tile(
    const ImageViewRGB32& before, const ImageViewRGB32& after,
    int dx, int dy
);


//  The Fly map of any region: blue title bar and white side borders.
//  (KantoMapDetector also checks for Kanto's land, so it misses the Sevii
//  Islands maps.)
class FlyMapDetector : public StaticScreenDetector{
public:
    FlyMapDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_top_box;
    ImageFloatBox m_left_box;
    ImageFloatBox m_right_box;
};
class FlyMapWatcher : public DetectorToFinder<FlyMapDetector>{
public:
    FlyMapWatcher(Color color)
        : DetectorToFinder("FlyMapWatcher", std::chrono::milliseconds(250), color)
    {}
};



}
}
}
#endif
