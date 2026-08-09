/*  BDSP Eye Blink Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EyeBlinkDetector_H
#define PokemonAutomation_PokemonBDSP_EyeBlinkDetector_H

#include <stddef.h>
#include <memory>
#include <string>
#include <vector>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Time.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class EyeBlinkDetector{
public:
    EyeBlinkDetector(std::shared_ptr<const ImageRGB32> open_eye, ImageFloatBox search_box);

    double match(const ImageViewRGB32& frame) const;

    const ImageFloatBox& search_box() const{ return m_search_box; }
    size_t template_width() const{ return m_width; }
    size_t template_height() const{ return m_height; }

private:
    std::shared_ptr<const ImageRGB32> m_template;
    size_t m_width;
    size_t m_height;
    std::vector<float> m_centered;
    double m_norm;
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
        std::shared_ptr<const ImageRGB32> open_eye,
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
