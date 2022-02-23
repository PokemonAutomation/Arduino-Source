/*  Shiny Sound Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_ShinySoundDetector_H
#define PokemonAutomation_PokemonLA_ShinySoundDetector_H

#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Inference/AudioInferenceCallback.h"
#include "CommonFramework/Inference/SpectrogramMatcher.h"

#include <memory>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class ShinySoundDetector : public AudioInferenceCallback{
public:
    ShinySoundDetector(Logger& logger);

    virtual bool process_spectrums(
        const std::vector<std::shared_ptr<AudioSpectrum>>& newSpectrums,
        AudioFeed& audioFeed
    ) override;

private:

    Logger& m_logger;

    std::unique_ptr<SpectrogramMatcher> m_matcher;
};



}
}
}
#endif
