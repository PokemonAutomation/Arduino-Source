/*  Shiny Sound Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_ShinySoundDetector_H
#define PokemonAutomation_PokemonLA_ShinySoundDetector_H

#include <functional>
#include <QImage>
#include "Common/Cpp/SpinLock.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/Inference/AudioPerSpectrumDetectorBase.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"

#include <memory>

namespace PokemonAutomation{

class ConsoleHandle;
class SpectrogramMatcher;

namespace NintendoSwitch{
namespace PokemonLA{


class ShinySoundDetector : public AudioPerSpectrumDetectorBase{
public:
    ShinySoundDetector(ConsoleHandle& console, bool stop_on_detected);

    ShinySoundResults results();

    // Implement AudioPerSpectrumDetectorBase::get_score_threshold()
    virtual float get_score_threshold() const override;

protected:
    // Implement AudioPerSpectrumDetectorBase::build_spectrogram_matcher()
    virtual std::unique_ptr<SpectrogramMatcher> build_spectrogram_matcher(size_t sampleRate) override;
};



//  New version that fires callback on shiny instead of setting internal state.
//  This one will fire multiple times if multiple shinies are detected with
//  sufficient time separation.
class ShinySoundDetector2 : public AudioInferenceCallback{
public:
    virtual ~ShinySoundDetector2();

    //  The return value of "on_shiny_callback" will be passed to the return
    //  value of "process_spectrums()". Thus returning true will stop the session.
    ShinySoundDetector2(
        ConsoleHandle& console,
        std::function<bool(float error_coefficient)> on_shiny_callback
    );

    void log_results();

    virtual bool process_spectrums(
        const std::vector<AudioSpectrum>& newSpectrums,
        AudioFeed& audioFeed
    ) override;

private:
    ConsoleHandle& m_console;
    std::function<bool(float error_coefficient)> m_on_shiny_callback;

    float m_lowest_error;

    WallClock m_last_timestamp;
    float m_last_error;
    bool m_last_reported;

    std::unique_ptr<SpectrogramMatcher> m_matcher;
};




}
}
}
#endif
