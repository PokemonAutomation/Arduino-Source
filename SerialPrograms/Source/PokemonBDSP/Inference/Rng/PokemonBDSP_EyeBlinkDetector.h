/*  BDSP Eye Blink Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EyeBlinkDetector_H
#define PokemonAutomation_PokemonBDSP_EyeBlinkDetector_H

#include <stddef.h>
#include <string>
#include <vector>
#include <opencv2/core/mat.hpp>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Time.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


//  Rescale a search region to the equivalent from a 1080p capture
const size_t BDSP_TEMPLATE_REFERENCE_HEIGHT = 1080;


class EyeBlinkDetector{
public:
    EyeBlinkDetector(const ImageViewRGB32& open_eye, ImageFloatBox search_box);

    //  Best normalized cross-correlation of the eye template
    //  https://en.wikipedia.org/wiki/Cross-correlation#Terminology_in_image_processing
    double match(const ImageViewRGB32& frame) const;

    const ImageFloatBox& search_box() const{ return m_search_box; }
    size_t template_width() const{ return (size_t)m_template.cols; }
    size_t template_height() const{ return (size_t)m_template.rows; }

private:
    cv::Mat m_template;
    ImageFloatBox m_search_box;
};


struct BlinkMatchSample{
    WallClock timestamp;
    double match;
};

struct BdspEyeTemplate{
    //  Path under the PokemonBDSP/Rng resource folder, scene subfolder included.
    std::string asset;
    ImageFloatBox box;
    std::string label;
};


class EyeBlinkWatcher : public VisualInferenceCallback{
public:
    EyeBlinkWatcher(
        std::string label,
        const ImageViewRGB32& open_eye,
        ImageFloatBox search_box,
        Color color = COLOR_RED
    );

    const std::string& label() const{ return m_label; }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    std::vector<BlinkMatchSample> samples() const;
    size_t sample_count() const;
    double last_match() const;

    void discard_before(WallClock keep_from);

private:
    std::string m_label;
    Color m_color;
    EyeBlinkDetector m_detector;

    mutable SpinLock m_lock;
    std::vector<BlinkMatchSample> m_samples;
    double m_last_match;
};


}
}
}
#endif
