/*  Area Zero Sky Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AreaZeroSkyDetector_H
#define PokemonAutomation_PokemonSV_AreaZeroSkyDetector_H

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
namespace NintendoSwitch{
namespace PokemonSV{



class AreaZeroSkyDetector : public StaticScreenDetector{
public:
    virtual void make_overlays(VideoOverlaySet& items) const override{}
    virtual bool detect(const ImageViewRGB32& screen) const override;

    bool detect(Kernels::Waterfill::WaterfillObject& object, const ImageViewRGB32& screen) const;
};


class AreaZeroSkyTracker : public AreaZeroSkyDetector, public VisualInferenceCallback{
public:
    AreaZeroSkyTracker(VideoOverlay& overlay);

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



}
}
}
#endif
