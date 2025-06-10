/*  Home Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_HomeMenuDetector_H
#define PokemonAutomation_NintendoSwitch_HomeMenuDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleState.h"
#include "NintendoSwitch_ConsoleTypeDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class HomeMenuDetector : public StaticScreenDetector{
public:
    HomeMenuDetector(ConsoleState& state, Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ConsoleTypeDetector_Home m_console_type;
    ImageFloatBox m_bottom_row;
    ImageFloatBox m_bottom_icons;
    ImageFloatBox m_bottom_left;
    ImageFloatBox m_bottom_right;
    ImageFloatBox m_user_icons;
    ImageFloatBox m_game_slot;
};
class HomeMenuWatcher : public DetectorToFinder<HomeMenuDetector>{
public:
    HomeMenuWatcher(
        ConsoleState& state,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("HomeMenuWatcher", hold_duration, state)
    {}
};




}
}
#endif
