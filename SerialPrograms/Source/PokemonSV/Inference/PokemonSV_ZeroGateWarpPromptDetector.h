/*  Zero Gate Warp Prompt Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_ZeroGateWarpPromptDetector_H
#define PokemonAutomation_PokemonSV_ZeroGateWarpPromptDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{



class ZeroGateWarpPromptDetector : public StaticScreenDetector{
public:
    ZeroGateWarpPromptDetector(Color color = COLOR_RED);
    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    int detect_location(const ImageViewRGB32& screen) const;

    bool move_cursor(
        const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
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
