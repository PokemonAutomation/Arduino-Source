/*  BDSP Eye Blink Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cmath>
#include <opencv2/imgproc.hpp>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageRGB32_OpenCV.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonBDSP_EyeBlinkDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


//  Returned when the frame cannot be matched at all. Callers read a high score
//  as "eye open", so a frame we can't measure never gets counted as a blink.
const double NO_MATCH = 1.0;


EyeBlinkDetector::EyeBlinkDetector(const ImageViewRGB32& open_eye, ImageFloatBox search_box)
    : m_search_box(search_box)
{
    if (!open_eye){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "EyeBlinkDetector: No eye template was supplied."
        );
    }

    cv::cvtColor(to_OpenCV_ref(open_eye), m_template, cv::COLOR_BGRA2GRAY);

    cv::Scalar mean;
    cv::Scalar stddev;
    cv::meanStdDev(m_template, mean, stddev);
    if (stddev[0] <= 0){
        //  A flat template correlates with everything equally and can never see a blink
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "EyeBlinkDetector: The eye template is a single flat colour."
        );
    }
}

double EyeBlinkDetector::match(const ImageViewRGB32& frame) const{
    if (!frame || frame.height() == 0){
        return NO_MATCH;
    }

    ImageViewRGB32 region = extract_box_reference(frame, m_search_box);
    if (!region){
        return NO_MATCH;
    }

    //  Rescale the region to the resolution the template was cropped at
    //  based on the capture height
    ImageRGB32 rescaled;
    if (frame.height() != BDSP_TEMPLATE_REFERENCE_HEIGHT){
        double scale = (double)BDSP_TEMPLATE_REFERENCE_HEIGHT / (double)frame.height();
        size_t width = (size_t)std::llround(region.width() * scale);
        size_t height = (size_t)std::llround(region.height() * scale);
        if (width == 0 || height == 0){
            return NO_MATCH;
        }
        rescaled = region.scale_to(width, height);
        region = rescaled;
    }

    if (region.width() < template_width() || region.height() < template_height()){
        return NO_MATCH;
    }

    //  Match the greyscale image to the template by zero-normalized cross correlation
    cv::Mat grey;
    cv::cvtColor(to_OpenCV_ref(region), grey, cv::COLOR_BGRA2GRAY);

    cv::Mat scores;
    cv::matchTemplate(grey, m_template, scores, cv::TM_CCOEFF_NORMED);

    double best = NO_MATCH;
    cv::minMaxLoc(scores, nullptr, &best);
    return best;
}



EyeBlinkWatcher::EyeBlinkWatcher(
    std::string label,
    const ImageViewRGB32& open_eye,
    ImageFloatBox search_box,
    Color color
)
    : VisualInferenceCallback(label)
    , m_label(std::move(label))
    , m_color(color)
    , m_detector(open_eye, search_box)
    , m_last_match(1.0)
{}

void EyeBlinkWatcher::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_detector.search_box(), m_label);
}

bool EyeBlinkWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    double value = m_detector.match(frame);
    WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
    m_samples.emplace_back(BlinkMatchSample{timestamp, value});
    m_last_match = value;
    //  Never stops the session
    return false;
}

std::vector<BlinkMatchSample> EyeBlinkWatcher::samples() const{
    ReadSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
    return m_samples;
}
size_t EyeBlinkWatcher::sample_count() const{
    ReadSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
    return m_samples.size();
}
double EyeBlinkWatcher::last_match() const{
    ReadSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
    return m_last_match;
}

void EyeBlinkWatcher::discard_before(WallClock keep_from){
    WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
    auto keep = std::lower_bound(
        m_samples.begin(), m_samples.end(), keep_from,
        [](const BlinkMatchSample& sample, WallClock cutoff){
            return sample.timestamp < cutoff;
        }
    );
    if (keep == m_samples.begin()){
        return;
    }
    m_samples.erase(m_samples.begin(), keep);
}




}
}
}
