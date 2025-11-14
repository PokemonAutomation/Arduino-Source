/*  Close Game Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_CloseGameDetector_H
#define PokemonAutomation_NintendoSwitch_CloseGameDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class CloseGameDetector : public StaticScreenDetector{
public:
    CloseGameDetector(ConsoleHandle& console, Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_top_box;
    ImageFloatBox m_left_box;
    ImageFloatBox m_close_game_text_row;
    
};
class CloseGameWatcher : public DetectorToFinder<CloseGameDetector>{
public:
    CloseGameWatcher(
        ConsoleHandle& console,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("CloseGameWatcher", hold_duration, console)
    {}
};




}
}
#endif
