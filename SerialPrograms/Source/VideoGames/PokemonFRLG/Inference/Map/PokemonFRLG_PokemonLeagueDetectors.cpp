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
    case EliteFourRoom::NONE:    return "unknown";
    case EliteFourRoom::LORELEI: return "Lorelei's room";
    case EliteFourRoom::BRUNO:   return "Bruno's room";
    case EliteFourRoom::AGATHA:  return "Agatha's room";
    case EliteFourRoom::LANCE:   return "Lance's room";
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
const std::array<RoomColor, 4> ROOM_COLORS{{
    {EliteFourRoom::LORELEI, {0.2501, 0.3353, 0.4146}},
    {EliteFourRoom::BRUNO,   {0.4038, 0.3891, 0.2070}},
    {EliteFourRoom::AGATHA,  {0.3316, 0.2716, 0.3968}},
    {EliteFourRoom::LANCE,   {0.2096, 0.3941, 0.3963}},
}};

//  Standard solid-color tolerance, to allow for capture card differences.
//  At this tolerance the three bluish rooms (Lorelei, Agatha, Lance) can't
//  be told apart from each other, only from Bruno's yellow room. So the
//  detector only checks for the room the program expects: the rooms are
//  always entered in the same order, and this confirms the player made it
//  into an Elite Four room after each door.
const double ROOM_MAX_DISTANCE = 0.20;
const double FLOOR_MAX_STDDEV_SUM = 60;     //  the floor has faint stripes
const double FLOOR_MIN_RGB_SUM = 250;       //  rules out fades to black

const FloatPixel& room_color(EliteFourRoom room){
    for (const RoomColor& item : ROOM_COLORS){
        if (item.room == room){
            return item.ratio;
        }
    }
    static const FloatPixel NO_COLOR(0, 0, 0);
    return NO_COLOR;
}

bool floor_matches(const ImageStats& stats, EliteFourRoom room){
    return room != EliteFourRoom::NONE
        && stats.average.sum() >= FLOOR_MIN_RGB_SUM
        && is_solid(stats, room_color(room), ROOM_MAX_DISTANCE, FLOOR_MAX_STDDEV_SUM);
}

//  The closest room within the tolerance, for error messages.
EliteFourRoom closest_room(const ImageStats& stats){
    if (stats.average.sum() < FLOOR_MIN_RGB_SUM || stats.stddev.sum() > FLOOR_MAX_STDDEV_SUM){
        return EliteFourRoom::NONE;
    }
    const FloatPixel ratio = stats.average / stats.average.sum();

    EliteFourRoom best = EliteFourRoom::NONE;
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


bool is_in_elite_four_room(const ImageViewRGB32& screen, EliteFourRoom room){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);
    return floor_matches(image_stats(extract_box_reference(game_screen, FLOOR_LEFT_BOX)), room)
        && floor_matches(image_stats(extract_box_reference(game_screen, FLOOR_RIGHT_BOX)), room);
}

EliteFourRoom read_elite_four_room(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);
    EliteFourRoom left = closest_room(image_stats(extract_box_reference(game_screen, FLOOR_LEFT_BOX)));
    EliteFourRoom right = closest_room(image_stats(extract_box_reference(game_screen, FLOOR_RIGHT_BOX)));
    return left == right ? left : EliteFourRoom::NONE;
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
    return is_in_elite_four_room(screen, m_room);
}



HallOfFameSavingDetector::HallOfFameSavingDetector(Color color)
    : m_color(color)
    //  Same edges as WhiteDialogDetector.
    , m_dialog_right_box(0.923385, 0.748077, 0.00615385, 0.204577)
    , m_dialog_top_box(0.0704615, 0.741846, 0.859077, 0.00623077)
    , m_dialog_bottom_box(0.0716923, 0.943308, 0.851692, 0.00934615)
    //  Plain light-blue Hall of Fame background (174, 190, 241) above the box.
    , m_background_box(0.30, 0.20, 0.40, 0.30)
    //  Dark blue-grey band (70, 78, 99) across the top of the screen.
    , m_top_band_box(0.10, 0.02, 0.80, 0.06)
{}
void HallOfFameSavingDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_dialog_right_box));
    items.add(m_color, GAME_BOX.inner_to_outer(m_dialog_top_box));
    items.add(m_color, GAME_BOX.inner_to_outer(m_dialog_bottom_box));
    items.add(m_color, GAME_BOX.inner_to_outer(m_background_box));
    items.add(m_color, GAME_BOX.inner_to_outer(m_top_band_box));
}
bool HallOfFameSavingDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);
    if (!is_white(extract_box_reference(game_screen, m_dialog_right_box))
        || !is_white(extract_box_reference(game_screen, m_dialog_top_box))
        || !is_white(extract_box_reference(game_screen, m_dialog_bottom_box))
    ){
        return false;
    }

    //  A large plain light-blue area, with a darker plain band of a similar
    //  hue above it. Overworld rooms with a dialogue box open fail the
    //  plainness checks (sprites, pillars and floor patterns), so the color
    //  tolerance can stay at the usual level.
    ImageStats background = image_stats(extract_box_reference(game_screen, m_background_box));
    ImageStats top_band = image_stats(extract_box_reference(game_screen, m_top_band_box));
    return is_solid(background, {0.2872, 0.3144, 0.3984}, 0.20, 40)
        && is_solid(top_band, {0.2825, 0.3150, 0.4025}, 0.20, 30)
        && top_band.average.sum() < background.average.sum() * 0.7;
}


}
}
}
