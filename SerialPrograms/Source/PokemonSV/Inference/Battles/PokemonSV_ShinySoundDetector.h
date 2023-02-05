/*  Shiny Sound Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_ShinySoundDetector_H
#define PokemonAutomation_PokemonSV_ShinySoundDetector_H

#include "CommonFramework/Inference/AudioPerSpectrumDetectorBase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class ShinySoundDetector : public AudioPerSpectrumDetectorBase{
public:
    ShinySoundDetector(ConsoleHandle& console, DetectedCallback detected_callback);

    virtual float get_score_threshold() const override;

protected:
    virtual std::unique_ptr<SpectrogramMatcher> build_spectrogram_matcher(size_t sampleRate) override;
};




}
}
}
#endif
