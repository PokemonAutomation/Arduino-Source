/*  Alpha Roar Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_AlphaRoarDetector_H
#define PokemonAutomation_PokemonLA_AlphaRoarDetector_H

#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/Audio/AudioPerSpectrumDetectorBase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class AlphaRoarDetector : public AudioPerSpectrumDetectorBase{
public:
    AlphaRoarDetector(VideoStream& stream, DetectedCallback detected_callback);

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
