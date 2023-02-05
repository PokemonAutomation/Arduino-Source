/*  Berry Tree Rustling Sound Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "CommonFramework/Inference/SpectrogramMatcher.h"
#include "CommonFramework/Inference/AudioTemplateCache.h"
#include "PokemonSwSh_BerryTreeRustlingSoundDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


BerryTreeRustlingSoundDetector::BerryTreeRustlingSoundDetector(
    Logger& logger, ConsoleHandle& console,
    DetectedCallback detected_callback,
    float threshold
)
    // Use a red as the detection color because the Levelball is made using red apricorns.
    : AudioPerSpectrumDetectorBase(logger, "BerryTreeRustlingSoundDetector", "Berry tree rustling sound", COLOR_RED, console, detected_callback)
    , m_threshold(threshold)
{}


float BerryTreeRustlingSoundDetector::get_score_threshold() const{
    return m_threshold;
}

std::unique_ptr<SpectrogramMatcher> BerryTreeRustlingSoundDetector::build_spectrogram_matcher(size_t sampleRate){
    return std::make_unique<SpectrogramMatcher>(
        AudioTemplateCache::instance().get_throw("PokemonSwSh/BerryTreeRustlingSound", sampleRate),
        SpectrogramMatcher::Mode::RAW, sampleRate,
        2000,
        1
    );
}



}
}
}
