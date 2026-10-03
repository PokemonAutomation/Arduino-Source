/*  Pokemon League Detectors
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <array>
#include <cstdio>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_PokemonLeagueDetectors.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


const char* elite_four_room_name(EliteFourRoom room){
    switch (room){
    case EliteFourRoom::none:    return "unknown";
    case EliteFourRoom::lorelei: return "Lorelei's room";
    case EliteFourRoom::bruno:   return "Bruno's room";
    case EliteFourRoom::agatha:  return "Agatha's room";
    case EliteFourRoom::lance:   return "Lance's room";
    }
    return "unknown";
}


namespace{

//  Plain floor inside the white battle rectangle, below the center line,
//  between the rectangle's side lines and the center circle. These spots
//  are a few game pixels from every edge, so small framing differences
//  between capture cards don't reach the lines, the circle or the pillar
//  shadows. Coordinates are relative to the game box.
const ImageFloatBox FLOOR_LEFT_BOX (0.356, 0.601, 0.034, 0.079);
const ImageFloatBox FLOOR_RIGHT_BOX(0.611, 0.601, 0.034, 0.079);

struct RoomColor{
    EliteFourRoom room;
    FloatPixel ratio;   //  floor color as r/g/b ratios (sum to 1)
};

//  Measured from screenshots of each room (floor colors in RGB:
//  Lorelei 134,180,223 / Bruno 219,211,112 / Agatha 194,159,233 / Lance 91,172,173).
//  The closest pair (Lorelei vs Lance) is 0.074 apart, so a 0.05 limit
//  leaves room for small capture-card color differences.
const std::array<RoomColor, 4> ROOM_COLORS{{
    {EliteFourRoom::lorelei, {0.2501, 0.3353, 0.4146}},
    {EliteFourRoom::bruno,   {0.4038, 0.3891, 0.2070}},
    {EliteFourRoom::agatha,  {0.3316, 0.2716, 0.3968}},
    {EliteFourRoom::lance,   {0.2096, 0.3941, 0.3963}},
}};
const double ROOM_MAX_DISTANCE = 0.05;
const double FLOOR_MAX_STDDEV_SUM = 60;     //  the floor has faint stripes
const double FLOOR_MIN_RGB_SUM = 250;       //  rules out fades to black

EliteFourRoom classify_floor(const ImageViewRGB32& image){
    ImageStats stats = image_stats(image);
    const double sum = stats.average.sum();
    if (sum < FLOOR_MIN_RGB_SUM || stats.stddev.sum() > FLOOR_MAX_STDDEV_SUM){
        return EliteFourRoom::none;
    }
    const FloatPixel ratio = stats.average / sum;

    EliteFourRoom best = EliteFourRoom::none;
    double best_distance = ROOM_MAX_DISTANCE;
    for (const RoomColor& item : ROOM_COLORS){
        double distance = euclidean_distance(ratio, item.ratio);
        if (distance <= best_distance){
            best_distance = distance;
            best = item.room;
        }
    }
    return best;
}

}


std::string describe_elite_four_floor(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);
    std::string ret;
    const char* sides[] = {"left", "right"};
    const ImageFloatBox* boxes[] = {&FLOOR_LEFT_BOX, &FLOOR_RIGHT_BOX};
    for (int i = 0; i < 2; i++){
        ImageStats stats = image_stats(extract_box_reference(game_screen, *boxes[i]));
        char buffer[160];
        std::snprintf(
            buffer, sizeof(buffer),
            "%s%s floor: rgb(%.0f, %.0f, %.0f), stddev sum %.1f",
            i == 0 ? "" : " | ", sides[i],
            stats.average.r, stats.average.g, stats.average.b, stats.stddev.sum()
        );
        ret += buffer;
    }
    return ret;
}


EliteFourRoom read_elite_four_room(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);
    EliteFourRoom left = classify_floor(extract_box_reference(game_screen, FLOOR_LEFT_BOX));
    EliteFourRoom right = classify_floor(extract_box_reference(game_screen, FLOOR_RIGHT_BOX));
    return left == right ? left : EliteFourRoom::none;
}


EliteFourRoomDetector::EliteFourRoomDetector(Color color, EliteFourRoom room)
    : m_color(color)
    , m_room(room)
{}
void EliteFourRoomDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(FLOOR_LEFT_BOX));
    items.add(m_color, GAME_BOX.inner_to_outer(FLOOR_RIGHT_BOX));
}
bool EliteFourRoomDetector::detect(const ImageViewRGB32& screen){
    return read_elite_four_room(screen) == m_room;
}



HallOfFameSavingDetector::HallOfFameSavingDetector(Color color)
    : m_color(color)
    //  Same edges as WhiteDialogDetector.
    , m_dialog_right_box(0.923385, 0.748077, 0.00615385, 0.204577)
    , m_dialog_top_box(0.0704615, 0.741846, 0.859077, 0.00623077)
    , m_dialog_bottom_box(0.0716923, 0.943308, 0.851692, 0.00934615)
    //  Plain light-blue Hall of Fame background (174, 190, 241) above the box.
    , m_background_box(0.30, 0.20, 0.40, 0.30)
{}
void HallOfFameSavingDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_dialog_right_box));
    items.add(m_color, GAME_BOX.inner_to_outer(m_dialog_top_box));
    items.add(m_color, GAME_BOX.inner_to_outer(m_dialog_bottom_box));
    items.add(m_color, GAME_BOX.inner_to_outer(m_background_box));
}
bool HallOfFameSavingDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);
    return is_white(extract_box_reference(game_screen, m_dialog_right_box))
        && is_white(extract_box_reference(game_screen, m_dialog_top_box))
        && is_white(extract_box_reference(game_screen, m_dialog_bottom_box))
        && is_solid(
            extract_box_reference(game_screen, m_background_box),
            {0.2872, 0.3144, 0.3984}, 0.04, 40
        );
}


}
}
}
