/*  Detect Home
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_DetectHome_H
#define PokemonAutomation_NintendoSwitch_DetectHome_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"

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
class HomeWatcher : public HomeDetector, public VisualInferenceCallback{
public:
    HomeWatcher();
    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;
};



class StartGameUserSelectDetector : public StaticScreenDetector{
public:
    StartGameUserSelectDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    ImageFloatBox m_bottom_row;
    ImageFloatBox m_bottom_icons;
    ImageFloatBox m_top_row;
    ImageFloatBox m_mid_row;
    ImageFloatBox m_user_slot;
};
class StartGameUserSelectWatcher : public StartGameUserSelectDetector, public VisualInferenceCallback{
public:
    StartGameUserSelectWatcher();
    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    size_t m_consecutive_detections = 0;
};


// Detect the Switch system update screen when you are about to enter a game from Switch Home screen
class UpdateMenuDetector : public StaticScreenDetector{
public:
    UpdateMenuDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    ImageFloatBox m_box_top;
    ImageFloatBox m_box_mid;
    ImageFloatBox m_top;
    ImageFloatBox m_left;
    ImageFloatBox m_bottom_solid;
    ImageFloatBox m_bottom_buttons;
};
class UpdateMenuWatcher : public UpdateMenuDetector, public VisualInferenceCallback{
public:
    UpdateMenuWatcher(bool invert);
    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    bool m_invert;
};









}
}
#endif

