/*  BDSP Eye Blink Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cmath>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonBDSP_EyeBlinkDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


static float to_grey(uint32_t pixel){
    float r = (float)((pixel >> 16) & 0xff);
    float g = (float)((pixel >> 8) & 0xff);
    float b = (float)(pixel & 0xff);
    return 0.299f * r + 0.587f * g + 0.114f * b;
}


EyeBlinkDetector::EyeBlinkDetector(std::shared_ptr<const ImageRGB32> open_eye, ImageFloatBox search_box)
    : m_template(std::move(open_eye))
    , m_width(0)
    , m_height(0)
    , m_norm(0)
    , m_search_box(search_box)
{
    if (m_template == nullptr || !*m_template){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "EyeBlinkDetector: No eye template was supplied."
        );
    }
    m_width = m_template->width();
    m_height = m_template->height();

    m_centered.resize(m_width * m_height);
    double sum = 0;
    for (size_t y = 0; y < m_height; y++){
        for (size_t x = 0; x < m_width; x++){
            float value = to_grey(m_template->pixel(x, y));
            m_centered[y * m_width + x] = value;
            sum += value;
        }
    }
    double mean = sum / (double)m_centered.size();
    for (float& value : m_centered){
        value = (float)(value - mean);
        m_norm += (double)value * value;
    }

    if (m_norm <= 0){
        //  A flat template correlates with everything equally and can never see a blink
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "EyeBlinkDetector: The eye template is a single flat colour."
        );
    }
}

double EyeBlinkDetector::match(const ImageViewRGB32& frame) const{
    ImageViewRGB32 region = extract_box_reference(frame, m_search_box);
    size_t region_width = region.width();
    size_t region_height = region.height();
    if (region_width < m_width || region_height < m_height){
        return 1.0;
    }

    std::vector<float> grey(region_width * region_height);
    for (size_t y = 0; y < region_height; y++){
        for (size_t x = 0; x < region_width; x++){
            grey[y * region_width + x] = to_grey(region.pixel(x, y));
        }
    }

    const double count = (double)(m_width * m_height);
    double best = -1.0;

    for (size_t dy = 0; dy + m_height <= region_height; dy++){
        for (size_t dx = 0; dx + m_width <= region_width; dx++){
            double sum = 0;
            double sum_squares = 0;
            double cross = 0;
            for (size_t y = 0; y < m_height; y++){
                const float* row = grey.data() + (dy + y) * region_width + dx;
                const float* tpl = m_centered.data() + y * m_width;
                for (size_t x = 0; x < m_width; x++){
                    double value = row[x];
                    sum += value;
                    sum_squares += value * value;
                    cross += tpl[x] * value;
                }
            }
            //  The template already has its mean removed, so the window's mean
            //  drops out of the numerator and only its variance is needed.
            double variance = sum_squares - sum * sum / count;
            if (variance <= 0){
                continue;
            }
            double score = cross / std::sqrt(variance * m_norm);
            if (score > best){
                best = score;
            }
        }
    }

    return best;
}



EyeBlinkWatcher::EyeBlinkWatcher(
    std::string label,
    std::shared_ptr<const ImageRGB32> open_eye,
    ImageFloatBox search_box,
    Color color
)
    : VisualInferenceCallback(label)
    , m_label(std::move(label))
    , m_color(color)
    , m_detector(std::move(open_eye), search_box)
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
