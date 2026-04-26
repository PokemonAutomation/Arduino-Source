/*  Gigantamax Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_GigantamaxDetector_H
#define PokemonAutomation_PokemonHome_GigantamaxDetector_H

#include <optional>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


class GigantamaxDetector : public StaticScreenDetector{
public:
    GigantamaxDetector(
        Color color,
        VideoOverlay* overlay,
        const ImageFloatBox& box
    );

    const ImageFloatBox& last_detected() const{ return m_last_detected; }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    const Color m_color;
    VideoOverlay* m_overlay;
    const ImageFloatBox m_box;

    ImageFloatBox m_last_detected;
    std::optional<OverlayBoxScope> m_last_detected_box;
};
class GigantamaxWatcher : public DetectorToFinder<GigantamaxDetector>{
public:
    GigantamaxWatcher(
        Color color,
        VideoOverlay* overlay,
        const ImageFloatBox& box,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
        : DetectorToFinder("GigantamaxWatcher", hold_duration, color, overlay, box)
    {}
};


}
}
}
#endif
