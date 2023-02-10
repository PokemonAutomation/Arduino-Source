/*  Shiny Sound Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Inference/SpectrogramMatcher.h"
#include "CommonFramework/Inference/AudioTemplateCache.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV_ShinySoundDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


ShinySoundDetector::ShinySoundDetector(ConsoleHandle& console, DetectedCallback detected_callback)
    // Use a yellow as the detection color because the shiny animation is yellow.
    : AudioPerSpectrumDetectorBase("ShinySoundDetector", "Shiny sound", COLOR_YELLOW, console, detected_callback)
{}
float ShinySoundDetector::get_score_threshold() const{
    return (float)GameSettings::instance().SHINY_SOUND_THRESHOLD2;
}
std::unique_ptr<SpectrogramMatcher> ShinySoundDetector::build_spectrogram_matcher(size_t sample_rate){
    return std::make_unique<SpectrogramMatcher>(
        "Shiny Sound",
        AudioTemplateCache::instance().get_throw("PokemonSV/ShinySound", sample_rate),
        SpectrogramMatcher::Mode::SPIKE_CONV, sample_rate,
        GameSettings::instance().SHINY_SOUND_LOW_FREQUENCY
    );
}



}
}
}
