/*  Shiny Sound Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Audio/SpectrogramMatcher.h"
#include "CommonTools/Audio/AudioTemplateCache.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP_ShinySoundDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


ShinySoundDetector::ShinySoundDetector(Logger& logger, DetectedCallback detected_callback)
    // Use a yellow as the detection color because the shiny animation is yellow.
    : AudioPerSpectrumDetectorBase(
        logger,
        "ShinySoundDetector",
        "Shiny sound",
        COLOR_YELLOW,
        detected_callback
    )
{}


float ShinySoundDetector::get_score_threshold() const{
    return (float)GameSettings::instance().SHINY_SOUND_THRESHOLD;
}

std::unique_ptr<SpectrogramMatcher> ShinySoundDetector::build_spectrogram_matcher(size_t sample_rate){
    return std::make_unique<SpectrogramMatcher>(
        "Shiny Sound",
        AudioTemplateCache::instance().get_throw("PokemonBDSP/ShinySound", sample_rate),
        SpectrogramMatcher::Mode::SPIKE_CONV, sample_rate,
        GameSettings::instance().SHINY_SOUND_LOW_FREQUENCY
    );
}



}
}
}
