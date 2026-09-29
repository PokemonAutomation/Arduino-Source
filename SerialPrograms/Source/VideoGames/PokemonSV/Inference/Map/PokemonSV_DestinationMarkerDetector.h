/*  Destination Marker Detector
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_DestinationMarkerDetector_H
#define PokemonAutomation_PokemonSV_DestinationMarkerDetector_H

#include <vector>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{

class VideoOverlaySet;
class VideoOverlay;
class OverlayBoxScope;

namespace NintendoSwitch{
namespace PokemonSV{

class DestinationMarkerDetector : public StaticScreenDetector{
public:
    DestinationMarkerDetector(Color color, const ImageFloatBox& box, bool check_yellow);
    virtual ~DestinationMarkerDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    std::vector<ImageFloatBox> detect_all(const ImageViewRGB32& screen) const;

    // for detecting the marker within the minimap
    // ImageFloatBox for detecting within the minimap {0.815, 0.645, 0.180, 0.320}. needs to be slightly higher up than the minimap since the marker will stick up past the minimap
    std::vector<ImageFloatBox> detect_all_yellow(const ImageViewRGB32& screen) const;

protected:
    Color m_color;
    ImageFloatBox m_box;
    bool m_check_yellow;
};



class DestinationMarkerWatcher : public DetectorToFinder<DestinationMarkerDetector>{
    public:
    DestinationMarkerWatcher(
        Color color,
        const ImageFloatBox& box,
        bool check_yellow = false,
        std::chrono::milliseconds duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("DestinationMarkerWatcher", std::chrono::milliseconds(250), color, box, check_yellow)
    {}
};




}
}
}
#endif
