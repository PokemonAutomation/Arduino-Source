/*  Detect Home
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_DetectHome_H
#define PokemonAutomation_NintendoSwitch_DetectHome_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class HomeDetector : public StaticScreenDetector{
public:
    HomeDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    ImageFloatBox m_bottom_row;
    ImageFloatBox m_bottom_icons;
    ImageFloatBox m_bottom_left;
    ImageFloatBox m_bottom_right;
    ImageFloatBox m_user_icons;
    ImageFloatBox m_game_slot;
};
class HomeWatcher : public DetectorToFinder<HomeDetector>{
public:
    HomeWatcher(std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250))
         : DetectorToFinder("HomeWatcher", hold_duration)
    {}
};



class StartGameUserSelectDetector : public StaticScreenDetector{
public:
    StartGameUserSelectDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    ImageFloatBox m_bottom_row;
    ImageFloatBox m_bottom_icons;
    ImageFloatBox m_top_row;
    ImageFloatBox m_mid_row;
    ImageFloatBox m_user_slot;
};
class StartGameUserSelectWatcher : public DetectorToFinder<StartGameUserSelectDetector>{
public:
    StartGameUserSelectWatcher(Color color = COLOR_RED)
         : DetectorToFinder("StartGameUserSelectWatcher", std::chrono::milliseconds(250), color)
    {}
};


//  Detect the Switch system update screen when you are about to enter a game from Switch Home screen
class UpdateMenuDetector : public StaticScreenDetector{
public:
    UpdateMenuDetector(Color color = COLOR_RED, bool invert = false);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    bool m_invert;
    ImageFloatBox m_box_top;
    ImageFloatBox m_box_mid;
    ImageFloatBox m_top;
    ImageFloatBox m_left;
    ImageFloatBox m_bottom_solid;
    ImageFloatBox m_bottom_buttons;
};
class UpdateMenuWatcher : public DetectorToFinder<UpdateMenuDetector>{
public:
    UpdateMenuWatcher(Color color = COLOR_RED, bool invert = false)
         : DetectorToFinder("UpdateMenuWatcher", std::chrono::milliseconds(250), color, invert)
    {}
};



//  Detect the "Checking if the software can be played..." menu.
class CheckOnlineDetector : public StaticScreenDetector{
public:
    CheckOnlineDetector(Color color = COLOR_RED, bool invert = false);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    bool m_invert;
    ImageFloatBox m_box_top;
    ImageFloatBox m_box_mid;
    ImageFloatBox m_top;
    ImageFloatBox m_left;
    ImageFloatBox m_bottom_solid;
    ImageFloatBox m_bottom_buttons;
};
class CheckOnlineWatcher : public DetectorToFinder<CheckOnlineDetector>{
public:
    CheckOnlineWatcher(Color color = COLOR_RED, bool invert = false)
         : DetectorToFinder("CheckOnlineWatcher", std::chrono::milliseconds(250), color, invert)
    {}
};


}
}
#endif

