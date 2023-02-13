/*  Area Zero Sky Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AreaZyroSkyDetector_H
#define PokemonAutomation_PokemonSV_AreaZyroSkyDetector_H

#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{
    class WaterfillObject;
}
}
class BotBaseContext;
class ProgramEnvironment;
class ConsoleHandle;
namespace NintendoSwitch{
namespace PokemonSV{



class AreaZyroSkyDetector : public StaticScreenDetector{
public:
    virtual void make_overlays(VideoOverlaySet& items) const override{}
    virtual bool detect(const ImageViewRGB32& screen) const override;

    bool detect(Kernels::Waterfill::WaterfillObject& object, const ImageViewRGB32& screen) const;
};


class AreaZyroSkyTracker : public AreaZyroSkyDetector, public VisualInferenceCallback{
public:
    AreaZyroSkyTracker(VideoOverlay& overlay);

    bool sky_location(double& x, double& y) const;

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    VideoOverlay& m_overlay;
    SpinLock m_lock;
    double m_center_x;
    double m_center_y;
    std::unique_ptr<OverlayBoxScope> m_box;
};


void find_and_center_on_sky(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context
);



}
}
}
#endif
