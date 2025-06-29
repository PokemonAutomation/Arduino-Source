/*  Shiny Sound Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLGPE_ShinySoundDetector_H
#define PokemonAutomation_PokemonLGPE_ShinySoundDetector_H

#include "CommonTools/Audio/AudioPerSpectrumDetectorBase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{


class ShinySoundDetector : public AudioPerSpectrumDetectorBase{
public:
    //  Warning: The callback will be called from the audio inference thread.
    ShinySoundDetector(Logger& logger, DetectedCallback detected_callback);

    // Implement AudioPerSpectrumDetectorBase::get_score_threshold()
    virtual float get_score_threshold() const override;

protected:
    // Implement AudioPerSpectrumDetectorBase::build_spectrogram_matcher()
    virtual std::unique_ptr<SpectrogramMatcher> build_spectrogram_matcher(size_t sample_rate) override;
};




}
}
}
#endif
