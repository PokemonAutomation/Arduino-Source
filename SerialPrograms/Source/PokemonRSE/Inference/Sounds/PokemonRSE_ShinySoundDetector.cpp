/*  Shiny Sound Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Audio/AudioTemplateCache.h"
#include "CommonTools/Audio/SpectrogramMatcher.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonRSE/PokemonRSE_Settings.h"
#include "PokemonRSE_ShinySoundDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{


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
        AudioTemplateCache::instance().get_throw("PokemonRSE/ShinySound", sample_rate),
        SpectrogramMatcher::Mode::SPIKE_CONV, sample_rate,
        GameSettings::instance().SHINY_SOUND_LOW_FREQUENCY
    );
}



}
}
}
