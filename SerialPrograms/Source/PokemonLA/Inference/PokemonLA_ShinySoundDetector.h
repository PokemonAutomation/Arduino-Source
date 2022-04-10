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
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"

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

    void log_results();

    bool detected() const{
        return m_detected.load(std::memory_order_acquire);
    }
    ShinySoundResults results();

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
    WallClock m_time_detected;

    float m_error_coefficient;
    QImage m_screenshot;

    std::unique_ptr<SpectrogramMatcher> m_matcher;
};



}
}
}
#endif
