/*  Alpha Roar Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Audio/SpectrogramMatcher.h"
#include "CommonTools/Audio/AudioTemplateCache.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA_AlphaRoarDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



AlphaRoarDetector::AlphaRoarDetector(VideoStream& stream, DetectedCallback detected_callback)
    // Use a purple as the detection color because the alpha symbol is red. To differentiate with the
    // detection color of alpha music, the roar (which is loud -> heavy -> darker color) uses purple.
    : AudioPerSpectrumDetectorBase(
        stream.logger(),
        "AlphaRoarDetector",
        "Alpha roar",
        COLOR_PURPLE,
        detected_callback
    )
{}

float AlphaRoarDetector::get_score_threshold() const{
    return (float)GameSettings::instance().ALPHA_ROAR_THRESHOLD;
}

std::unique_ptr<SpectrogramMatcher> AlphaRoarDetector::build_spectrogram_matcher(size_t sample_rate){
    const double low_frequency_filter = 100.0; // we don't match frequencies under 100.0 Hz
    return std::make_unique<SpectrogramMatcher>(
        "Alpha Roar",
        AudioTemplateCache::instance().get_throw("PokemonLA/AlphaRoar", sample_rate),
        SpectrogramMatcher::Mode::RAW, sample_rate,
        low_frequency_filter
    );
}




}
}
}
