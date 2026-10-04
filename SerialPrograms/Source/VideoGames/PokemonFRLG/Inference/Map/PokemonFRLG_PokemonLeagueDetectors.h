/*  Pokemon League Detectors
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Detectors for the Indigo Plateau Pokemon League:
 *    - Which Elite Four room the player is standing in (by floor color).
 *    - The Hall of Fame "Saving... Don't turn off the power." message.
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_PokemonLeagueDetectors_H
#define PokemonAutomation_PokemonFRLG_PokemonLeagueDetectors_H

#include <chrono>
#include <string>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


enum class EliteFourRoom{
    NONE,
    LORELEI,
    BRUNO,
    AGATHA,
    LANCE,
};
const char* elite_four_room_name(EliteFourRoom room);


//  Whether the floor color on both sides of the battle area matches the
//  given room. Only valid once the player has walked into the room (the
//  camera then shows the floor in the sample boxes).
bool is_in_elite_four_room(const ImageViewRGB32& screen, EliteFourRoom room);

//  The room whose floor color is closest to what's on screen, for error
//  messages. Returns EliteFourRoom::NONE when no room is close, including
//  fades and the lobby.
EliteFourRoom read_elite_four_room(const ImageViewRGB32& screen);

//  The measured floor colors, for the log when a room isn't recognized.
std::string describe_elite_four_floor(const ImageViewRGB32& screen);


//  Detects that the player is in a specific Elite Four room.
class EliteFourRoomDetector : public StaticScreenDetector{
public:
    EliteFourRoomDetector(Color color, EliteFourRoom room);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    EliteFourRoom m_room;
};
class EliteFourRoomWatcher : public DetectorToFinder<EliteFourRoomDetector>{
public:
    EliteFourRoomWatcher(Color color, EliteFourRoom room)
        : DetectorToFinder("EliteFourRoomWatcher", std::chrono::milliseconds(250), color, room)
    {}
};


//  The Hall of Fame "Saving... Don't turn off the power." message: a white
//  dialogue box over the plain light-blue Hall of Fame background. The game
//  finishes saving while this message is on screen, so once it disappears
//  it is safe to reset.
class HallOfFameSavingDetector : public StaticScreenDetector{
public:
    HallOfFameSavingDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_dialog_right_box;
    ImageFloatBox m_dialog_top_box;
    ImageFloatBox m_dialog_bottom_box;
    ImageFloatBox m_background_box;
    ImageFloatBox m_top_band_box;
};
class HallOfFameSavingWatcher : public DetectorToFinder<HallOfFameSavingDetector>{
public:
    HallOfFameSavingWatcher(Color color)
        : DetectorToFinder("HallOfFameSavingWatcher", std::chrono::milliseconds(100), color)
    {}
};
class HallOfFameSavingOverWatcher : public DetectorToFinder<HallOfFameSavingDetector>{
public:
    HallOfFameSavingOverWatcher(Color color)
        : DetectorToFinder("HallOfFameSavingOverWatcher", FinderType::GONE, std::chrono::milliseconds(100), color)
    {}
};


}
}
}
#endif
