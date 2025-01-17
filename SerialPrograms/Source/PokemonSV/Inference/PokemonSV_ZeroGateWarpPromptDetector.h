/*  Zero Gate Warp Prompt Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_ZeroGateWarpPromptDetector_H
#define PokemonAutomation_PokemonSV_ZeroGateWarpPromptDetector_H

#include "Common/Cpp/Color.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"

namespace PokemonAutomation{
    class BotBaseContext;
    struct ProgramInfo;
namespace NintendoSwitch{
    class ConsoleHandle;
namespace PokemonSV{



class ZeroGateWarpPromptDetector : public StaticScreenDetector{
public:
    ZeroGateWarpPromptDetector(Color color = COLOR_RED);
    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    int detect_location(const ImageViewRGB32& screen) const;

    bool move_cursor(
        const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
        int row
    ) const;

private:
    GradientArrowDetector m_arrow;
};
class ZeroGateWarpPromptWatcher : public DetectorToFinder<ZeroGateWarpPromptDetector>{
public:
    ZeroGateWarpPromptWatcher(Color color = COLOR_RED)
         : DetectorToFinder("ZeroGateWarpPromptWatcher", std::chrono::milliseconds(250), color)
    {}
};




}
}
}
#endif
