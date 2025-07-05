/*  Audio Template Cache
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cfloat>
#include <sstream>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/AudioPipeline/AudioFeed.h"
#include "SpectrogramMatcher.h"
#include "AudioPerSpectrumDetectorBase.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


//std::atomic<bool> force_detected(false);



AudioPerSpectrumDetectorBase::AudioPerSpectrumDetectorBase(
    Logger& logger,
    std::string label,
    std::string audio_name,
    Color detection_color,
    DetectedCallback detected_callback
)
    : AudioInferenceCallback(std::move(label))
    , m_audio_name(std::move(audio_name))
    , m_detection_color(detection_color)
    , m_logger(logger)
    , m_detected_callback(std::move(detected_callback))
    , m_start_timestamp(current_time())
    , m_spectrums_processed(0)
{}
void AudioPerSpectrumDetectorBase::set_detected_callback(DetectedCallback detected_callback){
    m_detected_callback = std::move(detected_callback);
}
AudioPerSpectrumDetectorBase::~AudioPerSpectrumDetectorBase(){
    try{
        log_results();
    }catch (...){}
}
void AudioPerSpectrumDetectorBase::throw_if_no_sound(std::chrono::milliseconds min_duration) const{
    if (m_start_timestamp + min_duration > current_time()){
        return;
    }
    if (m_lowest_error < 1.0){
        return;
    }
    if (m_spectrums_processed > 0){
        return;
    }
    throw UserSetupError(m_logger, "No sound detected.");
}

void AudioPerSpectrumDetectorBase::log_results(){
    if (m_last_timestamp != WallClock::min()){
        m_logger.log(m_audio_name + " detected! Error Coefficient = " + tostr_default(m_lowest_error), COLOR_BLUE);
    }else{
        m_logger.log(m_audio_name + " not detected. Error Coefficient = " + tostr_default(m_lowest_error), COLOR_PURPLE);
    }

#if 0
    if (m_lowest_error >= 1){
        std::stringstream ss;
        ss << "AudioPerSpectrumDetectorBase ended with error of 1.0: " << endl;
        ss << "Elapsed Time: "<< std::chrono::duration_cast<std::chrono::seconds>(current_time() - m_start_timestamp).count() << endl;
        ss << "m_spectrums_processed = "<< m_spectrums_processed << endl;
        for (const auto& error : m_errors){
            ss << "[" << error.first << ":" << error.second << "]";
        }
//        m_logger.log(ss.str(), COLOR_RED);
        cout << ss.str() << endl;
    }
 #endif
}

bool AudioPerSpectrumDetectorBase::process_spectrums(
    const std::vector<AudioSpectrum>& new_spectrums,
    AudioFeed& audio_feed
){
//    cout << m_audio_name << ": " << new_spectrums.size() << endl;
    if (new_spectrums.empty()){
//        cout << "No new spectrums" << endl;
        return false;
    }
//    cout << "New spectrums - " << new_spectrums.size() << endl;

    WallClock now = current_time();
    m_spectrums_processed += new_spectrums.size();

    //  Clear last detection.
    if (m_last_timestamp + std::chrono::milliseconds(1000) <= now){
        m_last_error = 1.0;
        m_last_reported = false;
    }

    const size_t sample_rate = new_spectrums[0].sample_rate;
    // Lazy initialization of the spectrogram matcher.
    if (m_matcher == nullptr || m_matcher->sample_rate() != sample_rate){
        m_logger.log("Loading spectrogram...");
        m_matcher = build_spectrogram_matcher(sample_rate);
    }

    // Feed spectrum one by one to the matcher:
    // new_spectrums are ordered from newest (largest timestamp) to oldest (smallest timestamp).
    // To feed the spectrum from old to new, we need to go through the vector in the reverse order:


//#define PA_DEBUG_FORCE_PLA_SOUND

#ifdef PA_DEBUG_FORCE_PLA_SOUND
    static int debug_count = 0;
    debug_count++;
    cout << "debug_count = " << debug_count << endl;
#endif
    
    bool found = false;
    const float threshold = get_score_threshold();
    for (auto it = new_spectrums.rbegin(); it != new_spectrums.rend(); it++){
        std::vector<AudioSpectrum> single_spectrum = {*it};
        const float matcher_score = m_matcher->match(single_spectrum);
        // std::cout << "error: " << matcherScore << std::endl;

        if (m_lowest_error < 1.0){
            m_errors.clear();
        }else{
            m_errors.emplace_back(matcher_score, now_to_filestring());
        }

        if (matcher_score == FLT_MAX){
//            cout << "Not enough history: " << m_lowest_error << endl;
            continue; // error or not enough spectrum history
        }
//        cout << "Matcher Score: " << matcher_score << endl;

        // Record the lowest error found during the run
        m_lowest_error = std::min(m_lowest_error, matcher_score);

        found = matcher_score <= threshold;

        uint64_t curStamp = m_matcher->latestTimestamp();

#ifdef PA_DEBUG_FORCE_PLA_SOUND
        if (debug_count % 300 > 300 - 5){
            found = true;
        }
#endif

        if (found){
            // Record the time of this match
            // To avoid detect the same audio multiple times, use m_last_error >= 1.0 to
            // make sure m_last_timestamp is only updated at the first match of the same audio.
            if (m_last_error >= 1.0){
                m_last_timestamp = now;
            }
            // m_last_error tracks the lowest error found by the current match.
            m_last_error = std::min(m_last_error, matcher_score);

            std::ostringstream os;
            os << m_audio_name << " found, score " << matcher_score << "/" << threshold << ", scale: " << m_matcher->lastMatchedScale();
            m_logger.log(os.str(), COLOR_BLUE);
            audio_feed.add_overlay(curStamp+1-m_matcher->numMatchedWindows(), curStamp+1, m_detection_color);

            // Since the target audio is found, no need to check detection on the rest of the spectrums in `new_spectrums`.

            // Tell m_matcher to skip the remaining spectrums so that if `process_spectrums()` gets
            // called again on a newer batch of spectrums, m_matcher is happy.
            m_matcher->skip(std::vector<AudioSpectrum>(
                new_spectrums.begin(),
                new_spectrums.begin() + std::distance(it + 1, new_spectrums.rend())
            ));

            // Skip the remaining spectrums.
            break;
        }
    }

//    if (force_detected){
//        return true;
//    }

    //  No shiny detected.
    if (m_last_error >= 1.0){
        return false;
    }

    //  Shiny detected, but haven't waited long enough to measure its magnitude.
    if (m_last_timestamp + std::chrono::milliseconds(200) > now){
        return false;
    }

    //  Already reported a target match within one second. Defer reporting anything.
    if (m_last_reported){
        return false;
    }

    //  No callback. Can't report.
    if (m_detected_callback == nullptr){
        return false;
    }

    m_last_reported = true;
    return m_detected_callback(m_last_error);
}

void AudioPerSpectrumDetectorBase::clear(){
    m_matcher->clear();
}






}
