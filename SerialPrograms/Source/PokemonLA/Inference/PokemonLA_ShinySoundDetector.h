/*  Shiny Sound Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_ShinySoundDetector_H
#define PokemonAutomation_PokemonLA_ShinySoundDetector_H

#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/Inference/AudioInferenceCallback.h"

#include <memory>

namespace PokemonAutomation{

class SpectrogramMatcher;

namespace NintendoSwitch{
namespace PokemonLA{


class ShinySoundDetector : public AudioInferenceCallback{
public:
    ShinySoundDetector(LoggerQt& logger, int sampleRate);

    virtual bool process_spectrums(
        const std::vector<std::shared_ptr<const AudioSpectrum>>& newSpectrums,
        AudioFeed& audioFeed
    ) override;

    // Clear internal data to be used on another audio stream.
    void clear();

private:

    LoggerQt& m_logger;

    std::unique_ptr<SpectrogramMatcher> m_matcher;
};



}
}
}
#endif
