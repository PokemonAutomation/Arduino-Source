/*  Overworld Readers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <cstdlib>
#include <algorithm>
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_OverworldReaders.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


namespace{

constexpr double GBA_WIDTH = 240;
constexpr double GBA_HEIGHT = 160;
constexpr int TILE = 16;
constexpr int PLAYER_TILE_LEFT = 112;
constexpr int PLAYER_TILE_TOP = 72;

//  The purple hair of the Cooltrainer sprites, about (108, 83, 187). Nothing
//  else around the Sevault Canyon trainers is this blue-purple. (The house's
//  blue windows have less red than green; the hair has more.)
bool is_hair(uint32_t pixel){
    int r = (pixel >> 16) & 0xff;
    int g = (pixel >> 8) & 0xff;
    int b = pixel & 0xff;
    return b > 140 && b - r > 45 && b - g > 65 && r >= g;
}

//  Sprite outlines are near black. Nothing on the canyon floor is.
bool is_outline(uint32_t pixel){
    int r = (pixel >> 16) & 0xff;
    int g = (pixel >> 8) & 0xff;
    int b = pixel & 0xff;
    return r + g + b < 120;
}

//  The purple part of the house roof (below its pale stripes, whose color
//  varies by capture). The rock, sand, grass and message boxes are never this
//  blue; purple hair is, but in short runs.
bool is_roof(uint32_t pixel){
    int r = (pixel >> 16) & 0xff;
    int g = (pixel >> 8) & 0xff;
    int b = pixel & 0xff;
    return b > 110 && b - g > 50 && r < 200;
}

//  The pixel at the center of a GBA pixel.
uint32_t gba_pixel(const ImageViewRGB32& game_screen, int x, int y){
    size_t px = (size_t)((x + 0.5) * (double)game_screen.width() / GBA_WIDTH);
    size_t py = (size_t)((y + 0.5) * (double)game_screen.height() / GBA_HEIGHT);
    return game_screen.pixel(px, py);
}

}


ImageFloatBox head_box_for_tile(const TileOffset& tile){
    //  A sprite's head sits on the tile above the one it stands on. The hair
    //  ends about 1 pixel above the tile's top edge. The box reaches higher to
    //  cover a sprite that is mid-jump.
    double left = PLAYER_TILE_LEFT + TILE * tile.dx + 2;
    double top  = PLAYER_TILE_TOP + TILE * tile.dy - 13;
    return ImageFloatBox(left / GBA_WIDTH, top / GBA_HEIGHT, 12 / GBA_WIDTH, 18 / GBA_HEIGHT);
}

bool head_box_on_screen(const TileOffset& tile){
    ImageFloatBox box = head_box_for_tile(tile);
    return box.x >= 0 && box.y >= 0 && box.x + box.width <= 1 && box.y + box.height <= 1;
}


SpriteSighting find_purple_hair(const ImageViewRGB32& frame, const std::vector<TileOffset>& tiles){
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    const double scale_y = (double)game_screen.height() / GBA_HEIGHT;

    SpriteSighting best;
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
            best.y = float_box.y * GBA_HEIGHT + (sum_y / (double)count) / scale_y;
        }
    }

    //  The hair covers about a third of the box; a few stray pixels don't count.
    best.found = best.score > 0.08;
    return best;
}


SpriteSighting find_sprite_feet(const ImageViewRGB32& frame, const std::vector<TileOffset>& tiles){
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    if (game_screen.width() == 0 || game_screen.height() == 0){
        return SpriteSighting();
    }

    SpriteSighting best;
    for (const TileOffset& tile : tiles){
        //  The tile's columns (without the 1-pixel edges), from 8 pixels
        //  above the tile (a standing sprite's legs) to its bottom edge.
        int left = PLAYER_TILE_LEFT + TILE * tile.dx + 1;
        int right = left + TILE - 2;
        int bottom = PLAYER_TILE_TOP + TILE * tile.dy + TILE;
        int top = bottom - TILE - 8;
        top = std::max(top, 0);
        bottom = std::min(bottom, (int)GBA_HEIGHT);
        if (left < 0 || right > (int)GBA_WIDTH || bottom <= top){
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
        for (int y = 0; y < (int)GBA_HEIGHT; y += 2){
            black += is_black(x, y) ? 1 : 0;
            total++;
        }
        return black * 4 >= total * 3;
    };
    auto row_black = [&](int y){
        int black = 0;
        int total = 0;
        for (int x = 0; x < (int)GBA_WIDTH; x += 2){
            black += is_black(x, y) ? 1 : 0;
            total++;
        }
        return black * 4 >= total * 3;
    };

    int left = 0;
    while (left < (int)GBA_WIDTH && column_black(left)){
        left++;
    }
    int right = 0;
    while (right < (int)GBA_WIDTH && column_black((int)GBA_WIDTH - 1 - right)){
        right++;
    }
    int top = 0;
    while (top < (int)GBA_HEIGHT && row_black(top)){
        top++;
    }
    int bottom = 0;
    while (bottom < (int)GBA_HEIGHT && row_black((int)GBA_HEIGHT - 1 - bottom)){
        bottom++;
    }
    //  All black (a fade) or no margins at all: not readable.
    if (left >= (int)GBA_WIDTH || top >= (int)GBA_HEIGHT){
        return ret;
    }

    //  The map's edges on screen, from where the player's tile is drawn:
    //    left edge   = 112 - 16 x          right edge  = 112 + 16 (width - x)
    //    top edge    =  72 - 16 y          bottom edge =  72 + 16 (floor_rows - y)
    //  A few pixels of slack cover a capture that crops the screen slightly.
    //  Margins under 6 pixels are taken as no margin.
    double fx;
    if (left >= 6){
        fx = (PLAYER_TILE_LEFT - left) / (double)TILE;
    }else if (right >= 6){
        fx = map_width - (GBA_WIDTH - right - PLAYER_TILE_LEFT) / TILE;
    }else{
        return ret;
    }
    double fy;
    if (top >= 6){
        fy = (PLAYER_TILE_TOP - top) / (double)TILE;
    }else if (bottom >= 6){
        fy = floor_rows - (GBA_HEIGHT - bottom - PLAYER_TILE_TOP) / TILE;
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


VsSeekerIcon find_vs_seeker_icon(const ImageViewRGB32& frame, int left, int top, int right, int bottom){
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    if (game_screen.width() == 0 || game_screen.height() == 0){
        return VsSeekerIcon::NONE;
    }
    left = std::max(left, 0);
    top = std::max(top, 0);
    right = std::min(right, (int)GBA_WIDTH);
    bottom = std::min(bottom, (int)GBA_HEIGHT);
    const int SIZE = 16;
    if (right - left < SIZE || bottom - top < SIZE){
        return VsSeekerIcon::NONE;
    }

    //  Classify each GBA pixel of the region:
    //    1 = navy (the "!!" marks, about (56, 56, 120))
    //    2 = light blue (the "X" bubble, about (100, 176, 216))
    //    3 = white (both bubbles)
    const int width = right - left;
    const int height = bottom - top;
    std::vector<uint8_t> kind((size_t)width * (size_t)height, 0);
    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            uint32_t pixel = gba_pixel(game_screen, left + x, top + y);
            int r = (pixel >> 16) & 0xff;
            int g = (pixel >> 8) & 0xff;
            int b = pixel & 0xff;
            uint8_t k = 0;
            if (b >= 90 && b <= 175 && r >= 25 && r <= 95 && g >= 25 && g <= 95 &&
                std::abs(r - g) < 25 && b - r > 35
            ){
                k = 1;  //  Navy: dark, gray-blue. (Hair shading is redder; pure blue lines are brighter.)
            }else if (g >= 150 && g - r > 45 && b > g + 15 && b > 180){
                k = 2;
            }else if (r > 215 && g > 215 && b > 235){
                k = 3;
            }
            kind[(size_t)y * (size_t)width + (size_t)x] = k;
        }
    }

    //  Slide a bubble-sized window. A bubble has plenty of white; then the
    //  "!!" has navy marks (about 25 pixels), and the "X" a light blue fill
    //  (about 80 pixels; the "!!" only has a thin light blue rim).
    size_t best_x = 0;
    size_t best_navy = 0;
    for (int y = 0; y + SIZE <= height; y += 2){
        for (int x = 0; x + SIZE <= width; x += 2){
            size_t counts[4] = {0, 0, 0, 0};
            for (int dy = 0; dy < SIZE; dy++){
                for (int dx = 0; dx < SIZE; dx++){
                    counts[kind[(size_t)(y + dy) * (size_t)width + (size_t)(x + dx)]]++;
                }
            }
            if (counts[3] < 30){
                continue;
            }
            best_navy = std::max(best_navy, counts[1]);
            if (counts[1] < 4){
                best_x = std::max(best_x, counts[2]);
            }
        }
    }
    if (best_navy >= 12){
        return VsSeekerIcon::READY;
    }
    if (best_x >= 45){
        return VsSeekerIcon::NOT_READY;
    }
    return VsSeekerIcon::NONE;
}


std::vector<VsSeekerIconSighting> find_vs_seeker_icons(const ImageViewRGB32& frame){
    std::vector<VsSeekerIconSighting> ret;
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    if (game_screen.width() == 0 || game_screen.height() == 0){
        return ret;
    }
    const int W = (int)GBA_WIDTH;
    const int H = (int)GBA_HEIGHT;
    const int SIZE = 16;

    //  Same pixel classes as find_vs_seeker_icon(): 1 navy, 2 light blue, 3 white.
    std::vector<uint8_t> kind((size_t)W * (size_t)H, 0);
    for (int y = 0; y < H; y++){
        for (int x = 0; x < W; x++){
            uint32_t pixel = gba_pixel(game_screen, x, y);
            int r = (pixel >> 16) & 0xff;
            int g = (pixel >> 8) & 0xff;
            int b = pixel & 0xff;
            uint8_t k = 0;
            if (b >= 90 && b <= 175 && r >= 25 && r <= 95 && g >= 25 && g <= 95 &&
                std::abs(r - g) < 25 && b - r > 35
            ){
                k = 1;
            }else if (g >= 150 && g - r > 45 && b > g + 15 && b > 180){
                k = 2;
            }else if (r > 215 && g > 215 && b > 235){
                k = 3;
            }
            kind[(size_t)y * (size_t)W + (size_t)x] = k;
        }
    }

    //  Windows that look like a bubble, strongest first.
    struct Candidate{
        int score;
        VsSeekerIcon icon;
        int x;
        int y;
    };
    std::vector<Candidate> candidates;
    for (int y = 0; y + SIZE <= H; y += 2){
        for (int x = 0; x + SIZE <= W; x += 2){
            int counts[4] = {0, 0, 0, 0};
            for (int dy = 0; dy < SIZE; dy++){
                const uint8_t* row = &kind[(size_t)(y + dy) * (size_t)W + (size_t)x];
                for (int dx = 0; dx < SIZE; dx++){
                    counts[row[dx]]++;
                }
            }
            if (counts[3] < 30){
                continue;
            }
            if (counts[1] >= 12){
                candidates.push_back({counts[1] * 4, VsSeekerIcon::READY, x + SIZE / 2, y + SIZE / 2});
            }else if (counts[1] < 4 && counts[2] >= 45){
                candidates.push_back({counts[2], VsSeekerIcon::NOT_READY, x + SIZE / 2, y + SIZE / 2});
            }
        }
    }
    std::sort(
        candidates.begin(), candidates.end(),
        [](const Candidate& a, const Candidate& b){ return a.score > b.score; }
    );
    //  One sighting per bubble: drop windows overlapping a stronger one.
    //  (Two trainers side by side have their bubbles 16 pixels apart.)
    for (const Candidate& c : candidates){
        bool taken = false;
        for (const VsSeekerIconSighting& s : ret){
            if (std::abs(s.x - c.x) < 14 && std::abs(s.y - c.y) < 14){
                taken = true;
                break;
            }
        }
        if (!taken){
            ret.push_back({c.icon, c.x, c.y});
        }
    }
    //  Many windows hold the whole "!!", and the first one found is off
    //  center. Center each on the bubble's pixels.
    for (VsSeekerIconSighting& s : ret){
        for (int iteration = 0; iteration < 2; iteration++){
            int sum_x = 0;
            int sum_y = 0;
            int count = 0;
            for (int y = std::max(0, s.y - SIZE / 2); y < std::min(H, s.y + SIZE / 2); y++){
                for (int x = std::max(0, s.x - SIZE / 2); x < std::min(W, s.x + SIZE / 2); x++){
                    if (kind[(size_t)y * (size_t)W + (size_t)x] != 0){
                        sum_x += x;
                        sum_y += y;
                        count++;
                    }
                }
            }
            if (count == 0){
                break;
            }
            s.x = (int)std::lround((double)sum_x / count);
            s.y = (int)std::lround((double)sum_y / count);
        }
    }
    return ret;
}


void ScreenOdometry::clear(){
    m_refs.clear();
}

namespace{
//  The screen at GBA resolution, in gray. Water is 0 (and real black 1):
//  water animates, so it is left out of comparisons.
std::vector<uint8_t> gray_screen(const ImageViewRGB32& frame){
    std::vector<uint8_t> ret;
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    if (game_screen.width() == 0 || game_screen.height() == 0){
        return ret;
    }
    ret.resize((size_t)GBA_WIDTH * (size_t)GBA_HEIGHT);
    for (int y = 0; y < (int)GBA_HEIGHT; y++){
        for (int x = 0; x < (int)GBA_WIDTH; x++){
            uint32_t pixel = gba_pixel(game_screen, x, y);
            int r = (int)((pixel >> 16) & 0xff);
            int g = (int)((pixel >> 8) & 0xff);
            int b = (int)(pixel & 0xff);
            bool water = b > 150 && b > r + 40 && b > g + 15;
            ret[(size_t)y * (size_t)GBA_WIDTH + (size_t)x] = water ? 0 : (uint8_t)std::max(1, (r + g + b) / 3);
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
//  little. (Not a median or mean difference: grass, sand and rock are made of
//  the same tiles repeated, so a screen shifted by a tile matches most of
//  itself. A percentage of clearly different pixels counts the places that
//  don't repeat, while video noise stays under the threshold.)
int screen_difference(const std::vector<uint8_t>& cur, const std::vector<uint8_t>& ref, int ox, int oy){
    const int W = (int)GBA_WIDTH;
    const int H = (int)GBA_HEIGHT;
    //  cur(x, y) shows what ref(x + 16 ox, y + 16 oy) showed.
    int x0 = std::max(0, -16 * ox);
    int x1 = std::min(W, W - 16 * ox);
    int y0 = std::max(0, -16 * oy);
    int y1 = std::min(H, H - 16 * oy);
    if ((x1 - x0) * (y1 - y0) * 4 < W * H){
        return -1;
    }
    uint32_t different = 0;
    uint32_t count = 0;
    for (int y = y0; y < y1; y += 2){
        int ry = y + 16 * oy;
        for (int x = x0; x < x1; x += 2){
            int rx = x + 16 * ox;
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
}

void ScreenOdometry::remember(const ImageViewRGB32& frame, int x, int y){
    std::vector<uint8_t> gray = gray_screen(frame);
    if (gray.empty()){
        return;
    }
    //  Replace a screen remembered at the same tile.
    for (Reference& ref : m_refs){
        if (ref.x == x && ref.y == y){
            ref.gray = std::move(gray);
            return;
        }
    }
    m_refs.push_back({x, y, std::move(gray)});
    //  Keep the newest 40.
    if (m_refs.size() > 40){
        m_refs.erase(m_refs.begin());
    }
}

bool ScreenOdometry::has_reference_near(int x, int y, int distance) const{
    for (const Reference& ref : m_refs){
        if (std::abs(ref.x - x) <= distance && std::abs(ref.y - y) <= distance){
            return true;
        }
    }
    return false;
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
    //  remembered screens that overlap it.
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

HouseRoof find_house_roof(const ImageViewRGB32& frame){
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    HouseRoof ret;
    if (game_screen.width() == 0 || game_screen.height() == 0){
        return ret;
    }

    //  For each row, where the first long run of roof pixels starts. The roof
    //  is 80 pixels wide; nothing else in the canyon has a run of 24.
    const int MIN_RUN = 24;
    std::vector<int> start_of_row((size_t)GBA_HEIGHT, -1);
    std::vector<int> starts;
    for (int y = 0; y < (int)GBA_HEIGHT; y++){
        int x = 0;
        while (x < (int)GBA_WIDTH){
            if (!is_roof(gba_pixel(game_screen, x, y))){
                x++;
                continue;
            }
            int run_start = x;
            while (x < (int)GBA_WIDTH && is_roof(gba_pixel(game_screen, x, y))){
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
    for (int y = 0; y < (int)GBA_HEIGHT; y++){
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
    const int HEIGHT = 29;
    const int EDGE = 6;
    bool whole = std::abs((bottom - top) - HEIGHT) <= 3;
    bool top_ok = whole || top >= EDGE;
    bool bottom_ok = whole || bottom <= (int)GBA_HEIGHT - EDGE;
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

    //  Sample both frames at GBA resolution (one pixel per GBA pixel).
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
    for (int y = 4; y < (int)GBA_HEIGHT - 4; y += 2){
        for (int x = 4; x < (int)GBA_WIDTH - 4; x += 2){
            int sx = x + shift_x;
            int sy = y + shift_y;
            if (sx < 0 || sy < 0 || sx >= (int)GBA_WIDTH || sy >= (int)GBA_HEIGHT){
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


FlyMapDetector::FlyMapDetector(Color)
    : m_top_box(0.010, 0.010, 0.500, 0.080)     //  blue (0, 122, 255)
    , m_left_box(0.069, 0.110, 0.025, 0.880)    //  white
    , m_right_box(0.902, 0.110, 0.025, 0.880)   //  white
{}
void FlyMapDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_left_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_right_box));
}
bool FlyMapDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);
    return is_solid(extract_box_reference(game_screen, m_top_box), {0.0000, 0.3236, 0.6764}, 0.25, 20)
        && is_white(extract_box_reference(game_screen, m_left_box))
        && is_white(extract_box_reference(game_screen, m_right_box));
}



}
}
}
