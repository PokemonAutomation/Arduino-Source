/*  Shiny Sound Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_ShinySoundDetector_H
#define PokemonAutomation_PokemonBDSP_ShinySoundDetector_H

#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Inference/AudioPerSpectrumDetectorBase.h"

#include <memory>

namespace PokemonAutomation{

class ConsoleHandle;
class SpectrogramMatcher;

namespace NintendoSwitch{
namespace PokemonBDSP{


class ShinySoundDetector : public AudioPerSpectrumDetectorBase{
public:
    //  Warning: The callback will be called from the audio inference thread.
    ShinySoundDetector(Logger& logger, ConsoleHandle& console, OnShinyCallback on_shiny_callback);

    // Implement AudioPerSpectrumDetectorBase::get_score_threshold()
    virtual float get_score_threshold() const override;

protected:
    // Implement AudioPerSpectrumDetectorBase::build_spectrogram_matcher()
    virtual std::unique_ptr<SpectrogramMatcher> build_spectrogram_matcher(size_t sampleRate) override;
};




}
}
}
#endif
