/*  Alpha Roar Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QString>
#include "CommonFramework/Inference/SpectrogramMatcher.h"
#include "CommonFramework/Inference/AudioTemplateCache.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA_AlphaRoarDetector.h"

#include <sstream>
#include <cfloat>
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



AlphaRoarDetector::AlphaRoarDetector(ConsoleHandle& console, OnShinyCallback on_shiny_callback)
    // Use a purple as the detection color because the alpha symbol is red. To differentiate with the
    // detection color of alpha music, the roar (which is loud -> heavy -> darker color) uses purple.
    : AudioPerSpectrumDetectorBase("AlphaRoarDetector", "Alpha roar", COLOR_PURPLE, console, on_shiny_callback)
{}

float AlphaRoarDetector::get_score_threshold() const{
    return (float)GameSettings::instance().ALPHA_ROAR_THRESHOLD;
}

std::unique_ptr<SpectrogramMatcher> AlphaRoarDetector::build_spectrogram_matcher(size_t sampleRate){
    const double lowFrequencyFilter = 100.0; // we don't match frequencies under 100.0 Hz
    return std::make_unique<SpectrogramMatcher>(
        AudioTemplateCache::instance().get_throw("PokemonLA/AlphaRoar", sampleRate),
        SpectrogramMatcher::Mode::RAW, sampleRate,
        lowFrequencyFilter
    );
}




}
}
}
