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


// Abstract base class for detecting white/bright objects in overworld using waterfill.
// Virtual methods to implement:
//    - process_object(): Called for each candidate object found by waterfill (required)
//    - finish(): Called after all objects processed for post-processing (optional)
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

    // return inference box color
    Color color() const{ return m_box_color; }
    // return lower bound thresholds for used color filters. Higher bounds are always 0xffffffff.
    const std::set<Color>& thresholds() const{ return m_thresholds; }

    // return detected locations
    const std::vector<ImagePixelBox>& detections() const{ return m_detections; }

    // clear stored detection results
    void clear(){ m_detections.clear(); }

    // To be implemented by derived class:
    // Called for each candidate waterfill object found by waterfill to check if it is a match
    // (in case the object is a single white piece) or part of a match (in case the target object
    // is made by multiple disjoint white parts). Store found objects/parts in `m_detections`.
    // Note due to multiple color filters with different thresholds, we may end up with multiple
    // versions of the same objects/parts stored in `m_detections` (an object can be picked by more
    // than one color filter). This is OK. We will use `finish()` to finalize detection, which can
    // call `merge_overlapping_boxes()` to deduplicate found objects.
    // - input_resolution: full screen resolution
    // - image: the image region on screen where the white object detection is run on.
    // - object: a waterfill object found by one of the color filters (one in `m_thresholds`) on
    //   the `image`.
    virtual void process_object(
        Resolution input_resolution,
        const ImageViewRGB32& image,
        const WaterfillObject& object
    ) = 0;

    // Optional method to be implemented by derived class:
    // Called after all objects are processed by process_object() to finalize detection.
    // Example use case:
    // To detect the player-placed white flag on the overworld, we need to detect the two white
    // components of the flag, the pole and the flag itself, separately as two waterfill object,
    // given to `process_object()`. So in `finish()` we find the pair of the white object
    // representing the pole and the flag based on their relative position and size to determine
    // a flag match.
    virtual void finish(const ImageViewRGB32& image){}

protected:
    Color m_box_color;  // inference box color
    std::set<Color> m_thresholds;  // lower bound thresholds for color filters
    std::vector<ImagePixelBox> m_detections; // detected locations
};


// A watcher to run a bunch of WhiteObjectDetectors to detect various white objects on screen.
class WhiteObjectWatcher : public VisualInferenceCallback{
public:
    // box: the region on the screen to run detection.
    // detectors: various WhiteObjectDetectors -> whether to let `process_frame()` return true
    //   when that detector finds a match.
    WhiteObjectWatcher(
        VideoOverlay& overlay,
        const ImageFloatBox& box,
        std::vector<std::pair<WhiteObjectDetector&, bool>> detectors
    );

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  Use the stored WhtieObjectDetectors to detect various white objects on screen by calling
    //  `find_overworld_white_objects()`.
    //  Return true if one of the detectors finds a match and its corresponding `bool` passed into
    //  the constructor is True.
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;


private:
    ImageFloatBox m_box;
    VideoOverlaySet m_overlays;

    std::vector<std::pair<WhiteObjectDetector&, bool>> m_detectors;
};


// Run a group of WhiteObjectDetectors with a combined waterfill session on input image.
// Each candiate waterfill object is passed to matching WhiteObjectDetectors by calling
// WhiteObjectDetector::process_object(). After waterill session, call each
// WhiteObjectDetector::finish() to finalize white object detection.
// - input_resolution: full screen resolution. Some WhiteObjectDetector may need this info.
// - detectors: various WhiteObjectDetectors passed to WhiteObjectWatcher. The "bool" data
//   is not used in this function.
// - image: a crop of the screen to run detection on.
void find_overworld_white_objects(
    Resolution input_resolution,
    const std::vector<std::pair<WhiteObjectDetector&, bool>>& detectors,
    const ImageViewRGB32& image
);




}
}
}
#endif
