/*  Item Drop Sound Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Audio/SpectrogramMatcher.h"
#include "CommonTools/Audio/AudioTemplateCache.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA_ItemDropSoundDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


ItemDropSoundDetector::ItemDropSoundDetector(VideoStream& stream, DetectedCallback detected_callback)
    // Use a green as the detection color because the shiny symbol in LA is green.
    : AudioPerSpectrumDetectorBase(
        stream.logger(),
        "ItemDropSoundDetector",
        "Item drop sound",
        COLOR_DARKGREEN,
        detected_callback
    )
{}


float ItemDropSoundDetector::get_score_threshold() const{
    return (float)GameSettings::instance().ITEM_DROP_SOUND_THRESHOLD;
}

std::unique_ptr<SpectrogramMatcher> ItemDropSoundDetector::build_spectrogram_matcher(size_t sample_rate){
    return std::make_unique<SpectrogramMatcher>(
        "Item Drop",
        AudioTemplateCache::instance().get_throw("PokemonLA/ItemDropSound", sample_rate),
        SpectrogramMatcher::Mode::SPIKE_CONV, sample_rate,
        GameSettings::instance().ITEM_DROP_SOUND_LOW_FREQUENCY
    );
}



}
}
}
