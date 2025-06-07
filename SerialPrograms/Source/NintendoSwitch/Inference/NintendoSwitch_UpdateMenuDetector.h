/*  Update Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_UpdateMenuDetector_H
#define PokemonAutomation_NintendoSwitch_UpdateMenuDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"
#include "NintendoSwitch_ConsoleTypeDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class UpdateMenuDetector_Switch1 : public StaticScreenDetector{
public:
    UpdateMenuDetector_Switch1(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    ImageFloatBox m_box_top;
    ImageFloatBox m_box_mid;
    ImageFloatBox m_top;
    ImageFloatBox m_left;
    ImageFloatBox m_bottom_solid;
    ImageFloatBox m_bottom_buttons;
};
class UpdateMenuDetector_Switch2 : public StaticScreenDetector{
public:
    UpdateMenuDetector_Switch2(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    ImageFloatBox m_box_top;
    ImageFloatBox m_box_mid;
    ImageFloatBox m_top;
    ImageFloatBox m_left;
    ImageFloatBox m_bottom_solid;
    ImageFloatBox m_bottom_buttons;
};





//  Detect the Switch system update screen when you are about to enter a game from Switch Home screen
class UpdateMenuDetector : public StaticScreenDetector{
public:
    UpdateMenuDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    ConsoleTypeDetector_Home m_type_detector;
    UpdateMenuDetector_Switch1 m_switch1;
    UpdateMenuDetector_Switch2 m_switch2;
};
class UpdateMenuWatcher : public DetectorToFinder<UpdateMenuDetector>{
public:
    UpdateMenuWatcher(Color color = COLOR_RED)
         : DetectorToFinder("UpdateMenuWatcher", std::chrono::milliseconds(250), color)
    {}
};






}
}
#endif
