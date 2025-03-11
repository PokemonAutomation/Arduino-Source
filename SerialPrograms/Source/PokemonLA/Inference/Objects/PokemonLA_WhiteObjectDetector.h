/*  White Object Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_WhiteObjectDetector_H
#define PokemonAutomation_PokemonLA_WhiteObjectDetector_H

#include <vector>
#include <set>
#include "Common/Compiler.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{
    class WaterfillObject;
}
}
namespace NintendoSwitch{
namespace PokemonLA{



class WhiteObjectDetector{
protected:
    using WaterfillObject = Kernels::Waterfill::WaterfillObject;

public:
    virtual ~WhiteObjectDetector() = default;
    WhiteObjectDetector(const WhiteObjectDetector&) = delete;
    void operator=(const WhiteObjectDetector&) = delete;

    // thresholds: thresholds for various filters. Each filter has a different threshold
    // to filter out candidate objects on an image. The thresholds specified here are the
    // min color thresholds. The max color thresholds for all filters are always 0xffffffff.
    WhiteObjectDetector(Color inference_box_color, std::set<Color> thresholds)
        : m_box_color(inference_box_color)
        , m_thresholds(std::move(thresholds))
    {}

    Color color() const{ return m_box_color; }
    const std::set<Color>& thresholds() const{ return m_thresholds; }

    const std::vector<ImagePixelBox>& detections() const{ return m_detections; }
    void clear(){ m_detections.clear(); }

    virtual void process_object(const ImageViewRGB32& image, const WaterfillObject& object) = 0;
    virtual void finish(const ImageViewRGB32& image){}

protected:
    void merge_heavily_overlapping(double tolerance = 0.2);

protected:
    Color m_box_color;
    std::set<Color> m_thresholds;
    std::vector<ImagePixelBox> m_detections;
};






void find_overworld_white_objects(
    const std::vector<std::pair<WhiteObjectDetector&, bool>>& detectors,
    const ImageViewRGB32& screen
);

class WhiteObjectWatcher : public VisualInferenceCallback{
public:
    WhiteObjectWatcher(
        VideoOverlay& overlay,
        const ImageFloatBox& box,
        std::vector<std::pair<WhiteObjectDetector&, bool>> detectors
    );

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  Return true if the inference session should stop.
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;


private:
    ImageFloatBox m_box;
    VideoOverlaySet m_overlays;

    std::vector<std::pair<WhiteObjectDetector&, bool>> m_detectors;
};






}
}
}
#endif
