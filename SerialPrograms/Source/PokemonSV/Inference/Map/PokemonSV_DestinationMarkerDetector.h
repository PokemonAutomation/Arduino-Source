/*  Destination Marker Detector
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_DestinationMarkerDetector_H
#define PokemonAutomation_PokemonSV_DestinationMarkerDetector_H

#include <vector>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"

namespace PokemonAutomation{

class VideoOverlaySet;
class VideoOverlay;
class OverlayBoxScope;

namespace NintendoSwitch{
namespace PokemonSV{

class DestinationMarkerDetector : public StaticScreenDetector{
public:
    DestinationMarkerDetector(Color color, const ImageFloatBox& box);
    virtual ~DestinationMarkerDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    std::vector<ImageFloatBox> detect_all(const ImageViewRGB32& screen) const;

protected:
    Color m_color;
    ImageFloatBox m_box;
};



class DestinationMarkerWatcher : public DetectorToFinder<DestinationMarkerDetector>{
    public:
    DestinationMarkerWatcher(
        Color color,
        const ImageFloatBox& box,
        std::chrono::milliseconds duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("DestinationMarkerWatcher", std::chrono::milliseconds(250), color, box)
    {}
};




}
}
}
#endif
