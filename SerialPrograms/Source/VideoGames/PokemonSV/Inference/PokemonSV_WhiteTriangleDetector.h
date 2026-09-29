/*  White Triangle Detector

 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_WhiteTriangleDetector_H
#define PokemonAutomation_PokemonSV_WhiteTriangleDetector_H

#include <vector>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Containers/FixedLimitVector.h"
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

class WhiteTriangleDetector : public StaticScreenDetector{
public:
    WhiteTriangleDetector(Color color, const ImageFloatBox& box);
    virtual ~WhiteTriangleDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

protected:
    Color m_color;
    ImageFloatBox m_box;
};



class WhiteTriangleWatcher : public DetectorToFinder<WhiteTriangleDetector>{
public:
    WhiteTriangleWatcher(
        Color color,
        const ImageFloatBox& box,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
        : DetectorToFinder("WhiteTriangleDetector", hold_duration, color, box)
    {}
};





}
}
}
#endif
