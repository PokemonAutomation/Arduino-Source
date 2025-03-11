/*  Item Drop Sound Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_ItemDropSoundDetector_H
#define PokemonAutomation_PokemonLA_ItemDropSoundDetector_H

#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/Audio/AudioPerSpectrumDetectorBase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class ItemDropSoundDetector : public AudioPerSpectrumDetectorBase{
public:
    //  Warning: The callback will be called from the audio inference thread.
    ItemDropSoundDetector(VideoStream& stream, DetectedCallback detected_callback);

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
