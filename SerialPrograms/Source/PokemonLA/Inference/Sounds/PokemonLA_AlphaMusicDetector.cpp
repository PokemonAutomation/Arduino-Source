/*  Alpha Music Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Audio/SpectrogramMatcher.h"
#include "CommonTools/Audio/AudioTemplateCache.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA_AlphaMusicDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



AlphaMusicDetector::AlphaMusicDetector(VideoStream& stream, DetectedCallback detected_callback)
    // Use a red as the detection color because the alpha symbol is red.
    : AudioPerSpectrumDetectorBase(
        stream.logger(),
        "AlphaMusicDetector",
        "Alpha music",
        COLOR_RED,
        detected_callback
    )
{}

float AlphaMusicDetector::get_score_threshold() const{
    return (float)GameSettings::instance().ALPHA_MUSIC_THRESHOLD;
}

std::unique_ptr<SpectrogramMatcher> AlphaMusicDetector::build_spectrogram_matcher(size_t sample_rate){
    const double low_frequency_filter = 50.0; // we don't match frequencies under 50.0 Hz
    const size_t templateSubdivision = 12;
    return std::make_unique<SpectrogramMatcher>(
        "Alpha Music",
        AudioTemplateCache::instance().get_throw("PokemonLA/AlphaMusic", sample_rate),
        SpectrogramMatcher::Mode::RAW, sample_rate,
        low_frequency_filter, templateSubdivision
    );
}




}
}
}
