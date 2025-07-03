/*  Start Game User-Select Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_StartGameUserSelectDetector_H
#define PokemonAutomation_NintendoSwitch_StartGameUserSelectDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"
#include "NintendoSwitch_ConsoleTypeDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




class StartGameUserSelectDetector_Switch1 : public StaticScreenDetector{
public:
    StartGameUserSelectDetector_Switch1(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_bottom_row;
    ImageFloatBox m_bottom_icons;
    ImageFloatBox m_top_row;
    ImageFloatBox m_mid_row;
    ImageFloatBox m_user_slot;
};
class StartGameUserSelectDetector_Switch2 : public StaticScreenDetector{
public:
    StartGameUserSelectDetector_Switch2(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_bottom_row;
    ImageFloatBox m_bottom_icons;
    ImageFloatBox m_top_row;
    ImageFloatBox m_mid_row;
    ImageFloatBox m_user_slot;
};





class StartGameUserSelectDetector : public StaticScreenDetector{
public:
    StartGameUserSelectDetector(ConsoleHandle& console, Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;
    virtual void commit_state() override;
    bool detect_only(const ImageViewRGB32& screen);

private:
    ConsoleHandle& m_console;
    ConsoleTypeDetector_StartGameUserSelect m_type_detector;
    StartGameUserSelectDetector_Switch1 m_switch1;
    StartGameUserSelectDetector_Switch2 m_switch2;

    ConsoleType m_console_type;
};
class StartGameUserSelectWatcher : public DetectorToFinder<StartGameUserSelectDetector>{
public:
    StartGameUserSelectWatcher(ConsoleHandle& console, Color color = COLOR_RED)
         : DetectorToFinder("StartGameUserSelectWatcher", std::chrono::milliseconds(250), console, color)
    {}
};






}
}
#endif
