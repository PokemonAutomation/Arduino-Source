/*  Catch Fanfare Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Audio/AudioTemplateCache.h"
#include "CommonTools/Audio/SpectrogramMatcher.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_CatchFanfareDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


CatchFanfareDetector::CatchFanfareDetector(Logger& logger, DetectedCallback detected_callback)
    : AudioPerSpectrumDetectorBase(
        logger,
        "CatchFanfareDetector",
        "Catch fanfare",
        COLOR_BLUE,
        detected_callback
    )
{}


float CatchFanfareDetector::get_score_threshold() const{
    return (float)GameSettings::instance().CATCH_FANFARE_THRESHOLD;
}

std::unique_ptr<SpectrogramMatcher> CatchFanfareDetector::build_spectrogram_matcher(size_t sample_rate){
    return std::make_unique<SpectrogramMatcher>(
        "Catch fanfare",
        AudioTemplateCache::instance().get_throw("PokemonFRLG/CatchFanfare", sample_rate),
        SpectrogramMatcher::Mode::SPIKE_CONV, sample_rate, 50
    );
}



}
}
}
