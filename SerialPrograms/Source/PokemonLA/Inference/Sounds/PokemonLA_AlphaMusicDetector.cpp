/*  Alpha Music Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "CommonFramework/Inference/SpectrogramMatcher.h"
#include "CommonFramework/Inference/AudioTemplateCache.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA_AlphaMusicDetector.h"

#include <sstream>
#include <cfloat>
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



AlphaMusicDetector::AlphaMusicDetector(Logger& logger, ConsoleHandle& console, DetectedCallback detected_callback)
    // Use a red as the detection color because the alpha symbol is red.
    : AudioPerSpectrumDetectorBase(logger, "AlphaMusicDetector", "Alpha music", COLOR_RED, console, detected_callback)
{}

float AlphaMusicDetector::get_score_threshold() const{
    return (float)GameSettings::instance().ALPHA_MUSIC_THRESHOLD;
}

std::unique_ptr<SpectrogramMatcher> AlphaMusicDetector::build_spectrogram_matcher(size_t sampleRate){
    const double lowFrequencyFilter = 50.0; // we don't match frequencies under 50.0 Hz
    const size_t templateSubdivision = 12;
    return std::make_unique<SpectrogramMatcher>(
        AudioTemplateCache::instance().get_throw("PokemonLA/AlphaMusic", sampleRate),
        SpectrogramMatcher::Mode::RAW, sampleRate,
        lowFrequencyFilter, templateSubdivision
    );
}




}
}
}
