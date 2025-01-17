/*  Berry Tree Rustling Sound Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "CommonTools/AudioTools/SpectrogramMatcher.h"
#include "CommonTools/AudioTools/AudioTemplateCache.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonSwSh_BerryTreeRustlingSoundDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


BerryTreeRustlingSoundDetector::BerryTreeRustlingSoundDetector(
    ConsoleHandle& console,
    DetectedCallback detected_callback,
    float threshold
)
    // Use a red as the detection color because the Levelball is made using red apricorns.
    : AudioPerSpectrumDetectorBase(
        console,
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
