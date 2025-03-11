/*  Shiny Sound Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_ShinySoundDetector_H
#define PokemonAutomation_PokemonSV_ShinySoundDetector_H

#include "CommonTools/Audio/AudioPerSpectrumDetectorBase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class ShinySoundDetector : public AudioPerSpectrumDetectorBase{
public:
    ShinySoundDetector(Logger& logger, DetectedCallback detected_callback);

    virtual float get_score_threshold() const override;

protected:
    virtual std::unique_ptr<SpectrogramMatcher> build_spectrogram_matcher(size_t sample_rate) override;
};




}
}
}
#endif
