/*  Audio Per-Spectrum Detector Base
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  A base class for any audio detector that tries to match an audio template starting at each 
 *  incoming spectrum in the audio stream (hence the "PerSpectrum").
 *  This means that, if called with three newest unseen spectrums from the audio feed, the detector
 *  will check whether the audio template matches audio starting at the first of the three spectrums,
 *  then check the second, and finally the third.
 *  Basically, it will try to match the audio template to any possible past location in the audio
 *  stream. So it will not miss any match if the audio template is short in duration and the matching
 *  algorithm needs to be very precise.
 *  This is opposite to the other design choice that only matches the audio starting at the newest
 *  spectrum when called. That design choice is better suited to non-time-critical audio matching and
 *  the matching algorithm can tolerate a few spectrums off on time axis.
 */

#ifndef PokemonAutomation_CommonFramework_AudioPerSpectrumDetectorBase_H
#define PokemonAutomation_CommonFramework_AudioPerSpectrumDetectorBase_H

#include <QImage>

#include <string>

#include "Common/Cpp/Color.h"
#include "Common/Cpp/SpinLock.h"
#include "Common/Cpp/Time.h"
#include "CommonFramework/InferenceInfra/AudioInferenceCallback.h"

namespace PokemonAutomation{

class ConsoleHandle;
class SpectrogramMatcher;

// A base class for audio detectors to match an audio template starting at each incoming spectrum in 
// the audio stream.
class AudioPerSpectrumDetectorBase : public AudioInferenceCallback{
public:
    // label: a name for this detector. Used for logging and profiling.
    // audio_name: the name of the audio to be detected (shiny sound etc). Capitalize first letter.
    // detection_color: the color to visualize the detection on audio spectrogram UI.
    AudioPerSpectrumDetectorBase(std::string label, std::string audio_name, Color detection_color,
        ConsoleHandle& console, bool stop_on_detected);

    virtual ~AudioPerSpectrumDetectorBase();

    // To be implemented by derived classes:
    // The match threshold for the target audio.
    virtual float get_score_threshold() const = 0;

    // Implement AudioInferenceCallback::process_spectrums()
    virtual bool process_spectrums(
        const std::vector<AudioSpectrum>& newSpectrums,
        AudioFeed& audioFeed
    ) override;

    // Clear internal data to be used on another audio stream.
    void clear();

    // Log whether the target audio is detected or not
    void log_results();

    bool detected() const{
        return m_detected.load(std::memory_order_acquire);
    }

protected:
    // To be implemented by derived classes:
    // build the actual spectrogram matcher for the target audio.
    virtual std::unique_ptr<SpectrogramMatcher> build_spectrogram_matcher(size_t sampleRate) = 0;

    // Name of the target audio to be detected. Used for logging.
    std::string m_audio_name;
    Color m_detection_color;
    
    ConsoleHandle& m_console;
    bool m_stop_on_detected = false;

    SpinLock m_lock;
    std::atomic<bool> m_detected;
    WallClock m_time_detected;

    float m_error_coefficient = 1.0f;
    QImage m_screenshot;

    std::unique_ptr<SpectrogramMatcher> m_matcher;
};





}
#endif
