/*  Berry Tree Rustling Sound Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BerryTreeRustlingSoundDetector_H
#define PokemonAutomation_PokemonSwSh_BerryTreeRustlingSoundDetector_H

#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/Audio/AudioPerSpectrumDetectorBase.h"

namespace PokemonAutomation{
    class SpectrogramMatcher;
namespace NintendoSwitch{
namespace PokemonSwSh{


class BerryTreeRustlingSoundDetector : public AudioPerSpectrumDetectorBase{
public:
    //  Warning: The callback will be called from the audio inference thread.
    BerryTreeRustlingSoundDetector(
        VideoStream& stream,
        DetectedCallback detected_callback,
        float threshold
    );

    // Implement AudioPerSpectrumDetectorBase::get_score_threshold()
    virtual float get_score_threshold() const override;

protected:
    // Implement AudioPerSpectrumDetectorBase::build_spectrogram_matcher()
    virtual std::unique_ptr<SpectrogramMatcher> build_spectrogram_matcher(size_t sample_rate) override;

    float m_threshold;
};




}
}
}
#endif
