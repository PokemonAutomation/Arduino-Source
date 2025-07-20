/*  Audio Per-Spectrum Detector Base
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  A base class for audio detectors.
 * 
 *  It implements AudioInferenceCallback so that it can hook to an inference session
 *  and stop the session when a target audio is detected.
 *  The inference session calls process_spectrums(...) periodically and stops when
 *  the function returns true.
 *  This base class implements this function.
 *  
 *  This base class is called "PerSpectrum" because it tries to match the audio template starting at
 *  each incoming spectrum in the audio stream.
 *  For example, if called with three newest unseen spectrums from the audio feed, (i.e `new_spectrums`
 *  contains three spectrums when passed to AudioPerSpectrumDetectorBase::process_spectrums(...))
 *  the detector will check whether the audio template matches the part of audio starting at the first
 *  of the three spectrums, then check starting at the second, and finally the third.
 *  Basically, it will try to match the audio template to any possible location in the current audio
 *  stream. So it will not miss any match even if the audio template is short in duration and the matching
 *  algorithm needs to be very precise.
 * 
 *  This is opposite to the other design choice that only matches the audio starting at the newest
 *  spectrum in `new_spectrums` when called. That design choice is better suited to non-time-critical
 *  audio matching and the matching algorithm can tolerate a few spectrums off on time axis.
 */

#ifndef PokemonAutomation_CommonTools_AudioPerSpectrumDetectorBase_H
#define PokemonAutomation_CommonTools_AudioPerSpectrumDetectorBase_H

#include <string>
#include <functional>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Time.h"
#include "CommonTools/InferenceCallbacks/AudioInferenceCallback.h"

namespace PokemonAutomation{


class Logger;
class SpectrogramMatcher;

// A virtual base class for audio detectors to match an audio template starting at each incoming
// spectrum in the audio stream.
// The derived classes need to implement two functions:
// - float get_score_threshold() const
// - std::unique_ptr<SpectrogramMatcher> build_spectrogram_matcher(size_t sample_rate)
class AudioPerSpectrumDetectorBase : public AudioInferenceCallback{
public:
    virtual ~AudioPerSpectrumDetectorBase();

    using DetectedCallback = std::function<bool(float error_coefficient)>;
    // label: a name for this detector. Used for logging and profiling.
    // audio_name: the name of the audio to be detected (shiny sound etc). Capitalize first letter.
    // detection_color: the color to visualize the detection on audio spectrogram UI.
    // on_shiny_callback(float error_coefficient) -> bool: 
    // when the detector finds a match, it calls this callback function to decide whether to stop
    // the inference session. The error coefficient of the found audio is passed to the callback
    // function. If it returns true, the inference session will stop (by returning true from 
    // AudioPerSpectrumDetectorBase::process_spectrums()).
    AudioPerSpectrumDetectorBase(
        Logger& logger,
        std::string label,
        std::string audio_name,
        Color detection_color,
        DetectedCallback detected_callback = nullptr
    );

    void set_detected_callback(DetectedCallback detected_callback);

    WallClock last_detection() const{
        return m_last_timestamp;
    }

    void throw_if_no_sound(std::chrono::milliseconds min_duration = std::chrono::milliseconds(5000)) const;

    // To be implemented by derived classes:
    // The match threshold for the target audio.
    virtual float get_score_threshold() const = 0;

    // Implement AudioInferenceCallback::process_spectrums()
    virtual bool process_spectrums(
        const std::vector<AudioSpectrum>& new_spectrums,
        AudioFeed& audio_feed
    ) override;

    // Clear internal data to be used on another audio stream.
    void clear();

    // Log whether the target audio is detected or not during the runtime of this detector.
    // This function will always be called when the detector is destructed.
    void log_results();

    float lowest_error() const{ return m_lowest_error; }

protected:
    // To be implemented by derived classes:
    // build the actual spectrogram matcher for the target audio.
    virtual std::unique_ptr<SpectrogramMatcher> build_spectrogram_matcher(size_t sample_rate) = 0;

    // Name of the target audio to be detected. Used for logging.
    std::string m_audio_name;
    // Color of the box to visualize the detection in the audio spectrogram UI.
    Color m_detection_color;
    
    Logger& m_logger;
    // Callback function to determine whether to stop the inference session when the target audio
    // is detected.
    DetectedCallback m_detected_callback;

    WallClock m_start_timestamp;
    uint64_t m_spectrums_processed;

    // Record lowest error coefficient (i.e best match) during the runtime of this detector.
    float m_lowest_error = 1.0f;
    // Record the last timestamp when the target audio is detected.
    // If no detection throughout the runtime, this will remain WallClock::min().
    WallClock m_last_timestamp = WallClock::min();
    // Record the last match error coefficient when the target audio is detected.
    // It will be reset to 1.0f when one second has passed since the last detection.
    float m_last_error = 1.0f;
    // This is set to true when a match is found. It will remain true for one second
    // so that the detector will not count the same detected audio multiple times.
    bool m_last_reported = false;
    
    std::unique_ptr<SpectrogramMatcher> m_matcher;

    std::vector<std::pair<float, std::string>> m_errors;
};





}
#endif
