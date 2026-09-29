/*  Berry Tree Rustling Sound Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "CommonTools/Audio/SpectrogramMatcher.h"
#include "CommonTools/Audio/AudioTemplateCache.h"
#include "PokemonSwSh_BerryTreeRustlingSoundDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


BerryTreeRustlingSoundDetector::BerryTreeRustlingSoundDetector(
    VideoStream& stream,
    DetectedCallback detected_callback,
    float threshold
)
    // Use a red as the detection color because the Levelball is made using red apricorns.
    : AudioPerSpectrumDetectorBase(
        stream.logger(),
        "BerryTreeRustlingSoundDetector",
        "Berry tree rustling sound",
        COLOR_RED,
        detected_callback
    )
    , m_threshold(threshold)
{}


float BerryTreeRustlingSoundDetector::get_score_threshold() const{
    return m_threshold;
}

std::unique_ptr<SpectrogramMatcher> BerryTreeRustlingSoundDetector::build_spectrogram_matcher(size_t sample_rate){
    return std::make_unique<SpectrogramMatcher>(
        "Berry Rustle",
        AudioTemplateCache::instance().get_throw("PokemonSwSh/BerryTreeRustlingSound", sample_rate),
        SpectrogramMatcher::Mode::RAW, sample_rate,
        2000,
        1
    );
}



}
}
}
