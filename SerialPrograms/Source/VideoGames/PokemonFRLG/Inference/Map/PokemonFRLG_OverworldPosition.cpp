/*  Overworld Position
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdlib.h>
#include <algorithm>
#include <cmath>
#include <sstream>
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "Tests/ImageUnitTest.h"
#include "Tests/TestUtils.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_OverworldGeometry.h"
#include "PokemonFRLG_OverworldPosition.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


namespace{

constexpr int W = GBA_SCREEN_WIDTH;
constexpr int H = GBA_SCREEN_HEIGHT;
constexpr int TILE = OVERWORLD_TILE_SIZE;

//  How many remembered screens ScreenOdometry keeps.
constexpr size_t MAX_REFERENCES = 40;

//  The screen at GBA resolution, in gray. Water is 0 (and real black 1):
//  water animates, so it is left out of comparisons.
std::vector<uint8_t> gray_screen(const ImageViewRGB32& frame){
    std::vector<uint8_t> ret;
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    if (game_screen.width() == 0 || game_screen.height() == 0){
        return ret;
    }
    ret.resize((size_t)W * (size_t)H);
    for (int y = 0; y < H; y++){
        for (int x = 0; x < W; x++){
            uint32_t pixel = gba_pixel(game_screen, x, y);
            int r = (int)((pixel >> 16) & 0xff);
            int g = (int)((pixel >> 8) & 0xff);
            int b = (int)(pixel & 0xff);
            bool water = b > 150 && b > r + 40 && b > g + 15;
            ret[(size_t)y * (size_t)W + (size_t)x] = water ? 0 : (uint8_t)std::max(1, (r + g + b) / 3);
        }
    }
    return ret;
}

//  The player (and the Surf mount) on the center tile changes with the
//  facing and animation; it isn't scenery.
bool in_player_box(int x, int y){
    return x >= 104 && x < 136 && y >= 40 && y < 100;
}

//  How different the current screen is from a remembered one if the player
//  stands (ox, oy) tiles from where it was remembered: the percentage of
//  pixels of the overlap that clearly differ, or -1 if they overlap too
//  little.
//
//  Not a median or mean difference: grass, sand and rock are made of the
//  same tiles repeated, so a screen shifted by a tile matches most of itself.
//  A percentage of clearly different pixels counts the places that don't
//  repeat, while video noise stays under the threshold.
int screen_difference(const std::vector<uint8_t>& cur, const std::vector<uint8_t>& ref, int ox, int oy){
    //  cur(x, y) shows what ref(x + 16 ox, y + 16 oy) showed.
    int x0 = std::max(0, -TILE * ox);
    int x1 = std::min(W, W - TILE * ox);
    int y0 = std::max(0, -TILE * oy);
    int y1 = std::min(H, H - TILE * oy);
    if ((x1 - x0) * (y1 - y0) * 4 < W * H){
        return -1;
    }
    uint32_t different = 0;
    uint32_t count = 0;
    for (int y = y0; y < y1; y += 2){
        int ry = y + TILE * oy;
        for (int x = x0; x < x1; x += 2){
            int rx = x + TILE * ox;
            if (in_player_box(x, y) || in_player_box(rx, ry)){
                continue;
            }
            uint8_t c = cur[(size_t)y * (size_t)W + (size_t)x];
            uint8_t r = ref[(size_t)ry * (size_t)W + (size_t)rx];
            if (c == 0 || r == 0){
                continue;   //  Water.
            }
            int d = (int)c - (int)r;
            if (d > 24 || d < -24){
                different++;
            }
            count++;
        }
    }
    //  Too little that isn't water to tell.
    if (count < 500){
        return -1;
    }
    return (int)(different * 100 / count);
}

//  The purple part of the Sevault Canyon house roof (below its pale stripes,
//  whose color varies by capture). The rock, sand, grass and message boxes
//  are never this blue; purple hair is, but in short runs.
bool is_roof(uint32_t pixel){
    int r = (pixel >> 16) & 0xff;
    int g = (pixel >> 8) & 0xff;
    int b = pixel & 0xff;
    return b > 110 && b - g > 50 && r < 200;
}

}


void ScreenOdometry::clear(){
    m_refs.clear();
}

void ScreenOdometry::remember(const ImageViewRGB32& frame, int x, int y){
    std::vector<uint8_t> gray = gray_screen(frame);
    if (gray.empty()){
        return;
    }
    for (Reference& ref : m_refs){
        if (ref.x == x && ref.y == y){
            ref.gray = std::move(gray);
            return;
        }
    }
    m_refs.push_back({x, y, std::move(gray)});
    if (m_refs.size() > MAX_REFERENCES){
        m_refs.erase(m_refs.begin());
    }
}

bool ScreenOdometry::has_reference_within(int x, int y, int dx, int dy) const{
    for (const Reference& ref : m_refs){
        if (std::abs(ref.x - x) <= dx && std::abs(ref.y - y) <= dy){
            return true;
        }
    }
    return false;
}

bool ScreenOdometry::locate(
    const ImageViewRGB32& frame,
    int guess_x, int guess_y, int radius,
    int& x, int& y, std::string& details,
    int radius_y
) const{
    if (radius_y < 0){
        radius_y = radius;
    }
    std::vector<uint8_t> cur = gray_screen(frame);
    if (cur.empty()){
        details = "no screen";
        return false;
    }

    //  For each candidate tile, the best (lowest) difference over the
    //  remembered screens that overlap it. A screen overlaps tiles up to 7
    //  across and 5 up or down from where it was remembered.
    int best = 256;
    int second = 256;
    int best_x = 0;
    int best_y = 0;
    for (int ty = guess_y - radius_y; ty <= guess_y + radius_y; ty++){
        for (int tx = guess_x - radius; tx <= guess_x + radius; tx++){
            int tile_best = 256;
            for (const Reference& ref : m_refs){
                int ox = tx - ref.x;
                int oy = ty - ref.y;
                if (std::abs(ox) > 7 || std::abs(oy) > 5){
                    continue;
                }
                int diff = screen_difference(cur, ref.gray, ox, oy);
                if (diff >= 0){
                    tile_best = std::min(tile_best, diff);
                }
            }
            if (tile_best < best){
                second = best;
                best = tile_best;
                best_x = tx;
                best_y = ty;
            }else if (tile_best < second){
                second = tile_best;
            }
        }
    }
    details = "best " + std::to_string(best) + "% at (" + std::to_string(best_x) + ", " +
        std::to_string(best_y) + "), next " + std::to_string(second) + "%";

    //  At the right tile only moving things differ (a few percent); at a
    //  wrong one, 20% or more. Mid-step, every tile is off.
    if (best > 12 || second < best + 8){
        return false;
    }
    x = best_x;
    y = best_y;
    return true;
}


IndoorTile read_indoor_position(const ImageViewRGB32& frame, int map_width, int floor_rows){
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    IndoorTile ret;
    if (game_screen.width() == 0 || game_screen.height() == 0){
        return ret;
    }

    auto is_black = [&](int x, int y){
        uint32_t pixel = gba_pixel(game_screen, x, y);
        int r = (pixel >> 16) & 0xff;
        int g = (pixel >> 8) & 0xff;
        int b = pixel & 0xff;
        return r < 40 && g < 40 && b < 40;
    };
    //  A column or row is outside the map if 3/4 of it is black (sprite
    //  outlines and the bottom row's door mat aren't enough to matter).
    auto column_black = [&](int x){
        int black = 0;
        int total = 0;
        for (int y = 0; y < H; y += 2){
            black += is_black(x, y) ? 1 : 0;
            total++;
        }
        return black * 4 >= total * 3;
    };
    auto row_black = [&](int y){
        int black = 0;
        int total = 0;
        for (int x = 0; x < W; x += 2){
            black += is_black(x, y) ? 1 : 0;
            total++;
        }
        return black * 4 >= total * 3;
    };

    int left = 0;
    while (left < W && column_black(left)){
        left++;
    }
    int right = 0;
    while (right < W && column_black(W - 1 - right)){
        right++;
    }
    int top = 0;
    while (top < H && row_black(top)){
        top++;
    }
    int bottom = 0;
    while (bottom < H && row_black(H - 1 - bottom)){
        bottom++;
    }
    //  All black (a fade): not readable.
    if (left >= W || top >= H){
        return ret;
    }

    //  The map's edges on screen, from where the player's tile is drawn:
    //    left edge   = 112 - 16 x          right edge  = 112 + 16 (width - x)
    //    top edge    =  72 - 16 y          bottom edge =  72 + 16 (floor_rows - y)
    //  Margins under 6 pixels are taken as no margin, which covers a capture
    //  that crops the screen slightly.
    double fx;
    if (left >= 6){
        fx = (PLAYER_TILE_LEFT - left) / (double)TILE;
    }else if (right >= 6){
        fx = map_width - (W - right - PLAYER_TILE_LEFT) / (double)TILE;
    }else{
        return ret;
    }
    double fy;
    if (top >= 6){
        fy = (PLAYER_TILE_TOP - top) / (double)TILE;
    }else if (bottom >= 6){
        fy = floor_rows - (H - bottom - PLAYER_TILE_TOP) / (double)TILE;
    }else{
        return ret;
    }

    //  Mid-step, the margins aren't whole tiles.
    double rx = std::round(fx);
    double ry = std::round(fy);
    if (std::abs(fx - rx) > 0.25 || std::abs(fy - ry) > 0.25){
        return ret;
    }
    ret.x = (int)rx;
    ret.y = (int)ry;
    ret.found = ret.x >= 0 && ret.x < map_width && ret.y >= 0 && ret.y < floor_rows;
    return ret;
}


HouseRoof find_house_roof(const ImageViewRGB32& frame){
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    HouseRoof ret;
    if (game_screen.width() == 0 || game_screen.height() == 0){
        return ret;
    }

    //  For each row, where the first long run of roof pixels starts. The roof
    //  is 80 pixels wide; nothing else in the canyon has a run of 24.
    const int MIN_RUN = 24;
    std::vector<int> start_of_row((size_t)H, -1);
    std::vector<int> starts;
    for (int y = 0; y < H; y++){
        int x = 0;
        while (x < W){
            if (!is_roof(gba_pixel(game_screen, x, y))){
                x++;
                continue;
            }
            int run_start = x;
            while (x < W && is_roof(gba_pixel(game_screen, x, y))){
                x++;
            }
            if (x - run_start >= MIN_RUN){
                start_of_row[(size_t)y] = run_start;
                starts.push_back(run_start);
                break;
            }
        }
    }
    if (starts.size() < 6){
        return ret;
    }

    //  The roof's left edge: the median start. Rows that start elsewhere
    //  (a window, a sprite next to the roof) aren't roof rows.
    std::sort(starts.begin(), starts.end());
    int left = starts[starts.size() / 2];
    if (left <= 0){
        return ret;     //  The edge is off screen: the column can't be told.
    }
    int top = -1;
    int bottom = -1;
    for (int y = 0; y < H; y++){
        if (start_of_row[(size_t)y] >= 0 && std::abs(start_of_row[(size_t)y] - left) <= 2){
            if (top < 0){
                top = y;
            }
            bottom = y + 1;
        }
    }

    //  The purple part is 29 rows tall. Near a screen edge the edge rows can
    //  be missed, so an edge within 6 rows of the screen's edge is only
    //  trusted if the height says the whole roof is there.
    const int ROOF_HEIGHT = 29;
    const int EDGE = 6;
    bool whole = std::abs((bottom - top) - ROOF_HEIGHT) <= 3;
    bool top_ok = whole || top >= EDGE;
    bool bottom_ok = whole || bottom <= H - EDGE;
    if (!whole && top_ok && bottom_ok){
        return ret;     //  Both edges in view but the wrong height: not the roof.
    }
    ret.found = top_ok || bottom_ok;
    ret.left = left;
    ret.top = top_ok ? top : -1;
    ret.bottom = bottom_ok ? bottom : -1;
    return ret;
}


bool screen_scrolled_one_tile(
    const ImageViewRGB32& before, const ImageViewRGB32& after,
    int dx, int dy
){
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    ImageViewRGB32 a = extract_box_reference(before, GAME_BOX);
    ImageViewRGB32 b = extract_box_reference(after, GAME_BOX);
    if (a.width() == 0 || a.height() == 0 || b.width() == 0 || b.height() == 0){
        return false;
    }

    auto gray = [](const ImageViewRGB32& image, int x, int y) -> int{
        uint32_t pixel = gba_pixel(image, x, y);
        return (int)(((pixel >> 16) & 0xff) + ((pixel >> 8) & 0xff) + (pixel & 0xff));
    };

    //  After a step right, the background seen at x was at x + 16 before.
    const int shift_x = TILE * dx;
    const int shift_y = TILE * dy;

    //  Skip the player (who stays put) and the bottom rows (dialog boxes),
    //  and only compare pixels that are on screen in both frames.
    auto skip = [](int x, int y){
        return (x >= 100 && x < 140 && y >= 40 && y < 96) || y >= 120;
    };

    double diff_still = 0;
    double diff_moved = 0;
    size_t count = 0;
    for (int y = 4; y < H - 4; y += 2){
        for (int x = 4; x < W - 4; x += 2){
            int sx = x + shift_x;
            int sy = y + shift_y;
            if (sx < 0 || sy < 0 || sx >= W || sy >= H){
                continue;
            }
            if (skip(x, y) || skip(sx, sy)){
                continue;
            }
            int now = gray(b, x, y);
            diff_still += std::abs(now - gray(a, x, y));
            diff_moved += std::abs(now - gray(a, sx, sy));
            count++;
        }
    }
    if (count == 0){
        return false;
    }
    diff_still /= (double)count;
    diff_moved /= (double)count;

    //  If nothing changed at all, the player did not move.
    if (diff_still < 6){
        return false;
    }
    return diff_moved < 0.6 * diff_still;
}


//  Two screens: the player standing still, then one step in the direction
//  (dx, dy). Remembering the first at (0, 0), the second must be located at
//  (dx, dy), and the first itself at (0, 0).
class Test_ScreenOdometry : public ImageUnitTest{
public:
    Test_ScreenOdometry(const std::string& before, const std::string& after, int dx, int dy)
        : ImageUnitTest("PokemonFRLG::ScreenOdometry - " + after, {before, after})
        , m_dx(dx), m_dy(dy)
    {
        m_memory += MAX_TEST_IMAGE_RGB32_BYTES;
    }

    virtual UnitTestResult run_on_images(
        Logger& logger, CancellableScope& scope,
        const std::vector<ImageRGB32>& images
    ) const override{
        ScreenOdometry odometry;
        odometry.remember(images[0], 0, 0);

        int x = -99;
        int y = -99;
        std::string details;
        if (!odometry.locate(images[0], 0, 0, 2, x, y, details) || x != 0 || y != 0){
            return "The remembered screen itself: " + details;
        }
        if (!odometry.locate(images[1], 0, 0, 2, x, y, details) || x != m_dx || y != m_dy){
            return "After the step: " + details + ", expected (" +
                std::to_string(m_dx) + ", " + std::to_string(m_dy) + ").";
        }
        return true;
    }

private:
    int m_dx;
    int m_dy;
};

void add_tests_ScreenOdometry(UnitTestDatabase& database){
    database.add<Test_ScreenOdometry>(
        "PokemonFRLG/Overworld/KindleRoad-Shore-Before.png",
        "PokemonFRLG/Overworld/KindleRoad-Shore-After.png",
        0, 1
    );
    database.add<Test_ScreenOdometry>(
        "PokemonFRLG/Overworld/KindleRoad-Sand-Before.png",
        "PokemonFRLG/Overworld/KindleRoad-Sand-After.png",
        0, 1
    );
}


//  The same pairs: the step was down, so the screen scrolled one tile for
//  (0, 1) and for no other direction, and comparing a screen with itself is
//  no step.
class Test_ScreenScrolledOneTile : public ImageUnitTest{
public:
    Test_ScreenScrolledOneTile(const std::string& before, const std::string& after, int dx, int dy)
        : ImageUnitTest("PokemonFRLG::ScreenScrolledOneTile - " + after, {before, after})
        , m_dx(dx), m_dy(dy)
    {
        m_memory += MAX_TEST_IMAGE_RGB32_BYTES;
    }

    virtual UnitTestResult run_on_images(
        Logger& logger, CancellableScope& scope,
        const std::vector<ImageRGB32>& images
    ) const override{
        const int DIRECTIONS[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
        for (const auto& d : DIRECTIONS){
            bool expected = d[0] == m_dx && d[1] == m_dy;
            if (screen_scrolled_one_tile(images[0], images[1], d[0], d[1]) != expected){
                return "Direction (" + std::to_string(d[0]) + ", " + std::to_string(d[1]) +
                    "): expected " + (expected ? "a step" : "no step") + ".";
            }
        }
        if (screen_scrolled_one_tile(images[0], images[0], m_dx, m_dy)){
            return "A screen compared with itself was taken for a step.";
        }
        return true;
    }

private:
    int m_dx;
    int m_dy;
};

void add_tests_ScreenScrolledOneTile(UnitTestDatabase& database){
    database.add<Test_ScreenScrolledOneTile>(
        "PokemonFRLG/Overworld/KindleRoad-Shore-Before.png",
        "PokemonFRLG/Overworld/KindleRoad-Shore-After.png",
        0, 1
    );
    database.add<Test_ScreenScrolledOneTile>(
        "PokemonFRLG/Overworld/KindleRoad-Sand-Before.png",
        "PokemonFRLG/Overworld/KindleRoad-Sand-After.png",
        0, 1
    );
}


//  Inside a house: the tile from the black margins.
class Test_IndoorPosition : public ImageUnitTest{
public:
    Test_IndoorPosition(const std::string& image, int map_width, int floor_rows, int x, int y)
        : ImageUnitTest("PokemonFRLG::IndoorPosition - " + image, {image})
        , m_map_width(map_width), m_floor_rows(floor_rows), m_x(x), m_y(y)
    {}

    virtual UnitTestResult run_on_images(
        Logger& logger, CancellableScope& scope,
        const std::vector<ImageRGB32>& images
    ) const override{
        IndoorTile tile = read_indoor_position(images[0], m_map_width, m_floor_rows);
        if (!tile.found){
            return "No tile found.";
        }
        TEST_RESULT_COMPONENT_EQUAL_STR(tile.x, m_x, "x");
        TEST_RESULT_COMPONENT_EQUAL_STR(tile.y, m_y, "y");
        return true;
    }

private:
    int m_map_width;
    int m_floor_rows;
    int m_x;
    int m_y;
};

void add_tests_IndoorPosition(UnitTestDatabase& database){
    //  The Sevault Canyon Chansey house: 11 tiles wide, 8 rows of floor.
    database.add<Test_IndoorPosition>("PokemonFRLG/Overworld/SevaultCanyon-ChanseyHouse.png", 11, 8, 1, 7);
}


//  The Sevault Canyon house roof: where it is, or that it isn't there.
class Test_HouseRoof : public ImageUnitTest{
public:
    Test_HouseRoof(const std::string& image, bool found, int left = 0, int top = 0, int bottom = 0)
        : ImageUnitTest("PokemonFRLG::HouseRoof - " + image, {image})
        , m_found(found), m_left(left), m_top(top), m_bottom(bottom)
    {}

    virtual UnitTestResult run_on_images(
        Logger& logger, CancellableScope& scope,
        const std::vector<ImageRGB32>& images
    ) const override{
        HouseRoof roof = find_house_roof(images[0]);
        TEST_RESULT_COMPONENT_EQUAL_STR(roof.found, m_found, "found");
        if (!m_found){
            return true;
        }
        if (std::abs(roof.left - m_left) > 1 || std::abs(roof.top - m_top) > 1 || std::abs(roof.bottom - m_bottom) > 1){
            return "Roof at left " + std::to_string(roof.left) + ", rows " + std::to_string(roof.top) +
                " to " + std::to_string(roof.bottom) + "; expected left " + std::to_string(m_left) +
                ", rows " + std::to_string(m_top) + " to " + std::to_string(m_bottom) + ".";
        }
        return true;
    }

private:
    bool m_found;
    int m_left;
    int m_top;
    int m_bottom;
};

void add_tests_HouseRoof(UnitTestDatabase& database){
    database.add<Test_HouseRoof>("PokemonFRLG/Overworld/SevaultCanyon-Cooltrainers.png", true, 129, 72, 101);
    database.add<Test_HouseRoof>("PokemonFRLG/Overworld/SevaultCanyon-VsSeekerReady.png", true, 129, 72, 101);
    database.add<Test_HouseRoof>("PokemonFRLG/Overworld/KindleRoad-Shore-Before.png", false);
    database.add<Test_HouseRoof>("PokemonFRLG/Overworld/KindleRoad-VsSeekerNotReady.png", false);
}


}
}
}
