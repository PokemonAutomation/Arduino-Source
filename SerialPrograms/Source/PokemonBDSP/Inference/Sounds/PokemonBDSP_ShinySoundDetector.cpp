/*  Shiny Sound Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QString>
#include "CommonFramework/Inference/SpectrogramMatcher.h"
#include "CommonFramework/Inference/AudioTemplateCache.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP_ShinySoundDetector.h"

#include <sstream>
#include <cfloat>
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


ShinySoundDetector::ShinySoundDetector(Logger& logger, ConsoleHandle& console, OnShinyCallback on_shiny_callback)
    // Use a yellow as the detection color because the shiny animation is yellow.
    : AudioPerSpectrumDetectorBase(logger, "ShinySoundDetector", "Shiny sound", COLOR_YELLOW, console, on_shiny_callback)
{}


float ShinySoundDetector::get_score_threshold() const{
    return (float)GameSettings::instance().SHINY_SOUND_THRESHOLD;
}

std::unique_ptr<SpectrogramMatcher> ShinySoundDetector::build_spectrogram_matcher(size_t sampleRate){
    return std::make_unique<SpectrogramMatcher>(
        AudioTemplateCache::instance().get_throw("PokemonBDSP/ShinySound", sampleRate),
        SpectrogramMatcher::Mode::SPIKE_CONV, sampleRate,
        GameSettings::instance().SHINY_SOUND_LOW_FREQUENCY
    );
}



}
}
}
