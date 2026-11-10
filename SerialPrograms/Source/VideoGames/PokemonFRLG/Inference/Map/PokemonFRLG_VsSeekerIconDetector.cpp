/*  VS Seeker Icon Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdlib.h>
#include <algorithm>
#include <cmath>
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_OverworldGeometry.h"
#include "PokemonFRLG_VsSeekerIconDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


namespace{

constexpr int ICON_SIZE = 16;

//  What each GBA pixel of a region looks like.
enum PixelKind : uint8_t{
    OTHER       = 0,
    NAVY        = 1,    //  The "!!" marks, about (56, 56, 120).
    LIGHT_BLUE  = 2,    //  The "X" bubble's fill, about (100, 176, 216).
    WHITE       = 3,    //  Both bubbles.
};

PixelKind classify(uint32_t pixel){
    int r = (pixel >> 16) & 0xff;
    int g = (pixel >> 8) & 0xff;
    int b = pixel & 0xff;
    //  Navy is dark gray-blue. (Hair shading is redder; pure blue lines are
    //  brighter.)
    if (b >= 90 && b <= 175 && r >= 25 && r <= 95 && g >= 25 && g <= 95 &&
        std::abs(r - g) < 25 && b - r > 35
    ){
        return NAVY;
    }
    if (g >= 150 && g - r > 45 && b > g + 15 && b > 180){
        return LIGHT_BLUE;
    }
    if (r > 215 && g > 215 && b > 235){
        return WHITE;
    }
    return OTHER;
}

//  The pixel kinds of a region of the GBA screen, row by row.
struct KindMap{
    int left = 0;
    int top = 0;
    int width = 0;
    int height = 0;
    std::vector<uint8_t> kinds;

    uint8_t at(int x, int y) const{
        return kinds[(size_t)(y - top) * (size_t)width + (size_t)(x - left)];
    }
};
KindMap classify_region(const ImageViewRGB32& game_screen, int left, int top, int right, int bottom){
    KindMap ret;
    ret.left = left;
    ret.top = top;
    ret.width = right - left;
    ret.height = bottom - top;
    ret.kinds.resize((size_t)ret.width * (size_t)ret.height);
    for (int y = top; y < bottom; y++){
        for (int x = left; x < right; x++){
            ret.kinds[(size_t)(y - top) * (size_t)ret.width + (size_t)(x - left)] =
                classify(gba_pixel(game_screen, x, y));
        }
    }
    return ret;
}

//  Whether the icon-sized window with top-left corner (x, y) looks like an
//  icon, and which. A bubble has plenty of white; then the "!!" has navy
//  marks (about 25 pixels), and the "X" a light blue fill (about 80 pixels;
//  the "!!" only has a thin light blue rim). `score` ranks the windows.
VsSeekerIcon classify_window(const KindMap& map, int x, int y, int& score){
    int counts[4] = {0, 0, 0, 0};
    for (int dy = 0; dy < ICON_SIZE; dy++){
        for (int dx = 0; dx < ICON_SIZE; dx++){
            counts[map.at(x + dx, y + dy)]++;
        }
    }
    if (counts[WHITE] < 30){
        return VsSeekerIcon::NONE;
    }
    if (counts[NAVY] >= 12){
        score = counts[NAVY] * 4;
        return VsSeekerIcon::READY;
    }
    if (counts[NAVY] < 4 && counts[LIGHT_BLUE] >= 45){
        score = counts[LIGHT_BLUE];
        return VsSeekerIcon::NOT_READY;
    }
    return VsSeekerIcon::NONE;
}

}


VsSeekerIcon find_vs_seeker_icon(const ImageViewRGB32& frame, int left, int top, int right, int bottom){
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    if (game_screen.width() == 0 || game_screen.height() == 0){
        return VsSeekerIcon::NONE;
    }
    left = std::max(left, 0);
    top = std::max(top, 0);
    right = std::min(right, GBA_SCREEN_WIDTH);
    bottom = std::min(bottom, GBA_SCREEN_HEIGHT);
    if (right - left < ICON_SIZE || bottom - top < ICON_SIZE){
        return VsSeekerIcon::NONE;
    }

    KindMap map = classify_region(game_screen, left, top, right, bottom);
    bool not_ready = false;
    for (int y = top; y + ICON_SIZE <= bottom; y += 2){
        for (int x = left; x + ICON_SIZE <= right; x += 2){
            int score = 0;
            VsSeekerIcon icon = classify_window(map, x, y, score);
            if (icon == VsSeekerIcon::READY){
                return VsSeekerIcon::READY;
            }
            if (icon == VsSeekerIcon::NOT_READY){
                not_ready = true;
            }
        }
    }
    return not_ready ? VsSeekerIcon::NOT_READY : VsSeekerIcon::NONE;
}


std::vector<VsSeekerIconSighting> find_vs_seeker_icons(const ImageViewRGB32& frame){
    std::vector<VsSeekerIconSighting> ret;
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    if (game_screen.width() == 0 || game_screen.height() == 0){
        return ret;
    }
    const int W = GBA_SCREEN_WIDTH;
    const int H = GBA_SCREEN_HEIGHT;
    KindMap map = classify_region(game_screen, 0, 0, W, H);

    //  Windows that look like an icon, strongest first.
    struct Candidate{
        int score;
        VsSeekerIcon icon;
        int x;
        int y;
    };
    std::vector<Candidate> candidates;
    for (int y = 0; y + ICON_SIZE <= H; y += 2){
        for (int x = 0; x + ICON_SIZE <= W; x += 2){
            int score = 0;
            VsSeekerIcon icon = classify_window(map, x, y, score);
            if (icon != VsSeekerIcon::NONE){
                candidates.push_back({score, icon, x + ICON_SIZE / 2, y + ICON_SIZE / 2});
            }
        }
    }
    std::sort(
        candidates.begin(), candidates.end(),
        [](const Candidate& a, const Candidate& b){
            return a.score > b.score;
        }
    );

    //  One sighting per icon: drop windows overlapping a stronger one. (Two
    //  trainers side by side have their icons 16 pixels apart.)
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

    //  Many windows hold the whole icon, and the strongest one can be off
    //  center. Center each sighting on the icon's pixels.
    for (VsSeekerIconSighting& s : ret){
        for (int iteration = 0; iteration < 2; iteration++){
            int sum_x = 0;
            int sum_y = 0;
            int count = 0;
            for (int y = std::max(0, s.y - ICON_SIZE / 2); y < std::min(H, s.y + ICON_SIZE / 2); y++){
                for (int x = std::max(0, s.x - ICON_SIZE / 2); x < std::min(W, s.x + ICON_SIZE / 2); x++){
                    if (map.at(x, y) != OTHER){
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



}
}
}
