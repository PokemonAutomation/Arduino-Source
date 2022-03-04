/*  Shiny Sound Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_ShinySoundDetector_H
#define PokemonAutomation_PokemonLA_ShinySoundDetector_H

#include <QImage>
#include "Common/Cpp/SpinLock.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/InferenceInfra/AudioInferenceCallback.h"

#include <memory>

namespace PokemonAutomation{

class ConsoleHandle;
class SpectrogramMatcher;

namespace NintendoSwitch{
namespace PokemonLA{


class ShinySoundDetector : public AudioInferenceCallback{
public:
    virtual ~ShinySoundDetector();
    ShinySoundDetector(ConsoleHandle& console, bool stop_on_detected);

    bool detected() const{
        return m_detected.load(std::memory_order_acquire);
    }
    QImage consume_screenshot();

    virtual bool process_spectrums(
        const std::vector<AudioSpectrum>& newSpectrums,
        AudioFeed& audioFeed
    ) override;

    // Clear internal data to be used on another audio stream.
    void clear();

private:
    ConsoleHandle& m_console;
    bool m_stop_on_detected;

    SpinLock m_lock;
    std::atomic<bool> m_detected;
    QImage m_screenshot;

    std::unique_ptr<SpectrogramMatcher> m_matcher;
};



}
}
}
#endif
