/*  Audio Template Cache
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cfloat>
#include <sstream>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/AudioPipeline/AudioFeed.h"
#include "CommonFramework/AudioPipeline/AudioTemplate.h"
#include "CommonFramework/Inference/SpectrogramMatcher.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "AudioPerSpectrumDetectorBase.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


AudioPerSpectrumDetectorBase::AudioPerSpectrumDetectorBase(
    Logger& logger,
    std::string label, std::string audio_name, Color detection_color,
    ConsoleHandle& console, OnShinyCallback on_shiny_callback
)
    : AudioInferenceCallback(std::move(label))
    , m_logger(logger)
    , m_audio_name(std::move(audio_name))
    , m_detection_color(detection_color)
    , m_console(console)
    , m_on_shiny_callback(std::move(on_shiny_callback))
    , m_start_timestamp(current_time())
{}
AudioPerSpectrumDetectorBase::~AudioPerSpectrumDetectorBase(){
    try{
        log_results();
    }catch (...){}
}
void AudioPerSpectrumDetectorBase::throw_if_no_sound(std::chrono::milliseconds min_duration){
    if (m_start_timestamp + min_duration > current_time()){
        return;
    }
    if (m_lowest_error < 1.0){
        return;
    }
    throw UserSetupError(m_logger, "No sound detected.");
}

void AudioPerSpectrumDetectorBase::log_results(){
    if (m_last_timestamp != WallClock::min()){
        m_console.log(m_audio_name + " detected! Error Coefficient = " + tostr_default(m_lowest_error), COLOR_BLUE);
    }else{
        m_console.log(m_audio_name + " not detected. Error Coefficient = " + tostr_default(m_lowest_error), COLOR_PURPLE);
    }
}

bool AudioPerSpectrumDetectorBase::process_spectrums(
    const std::vector<AudioSpectrum>& newSpectrums,
    AudioFeed& audioFeed
){
    if (newSpectrums.empty()){
        return false;
    }

    WallClock now = current_time();

    //  Clear last detection.
    if (m_last_timestamp + std::chrono::milliseconds(1000) <= now){
        m_last_error = 1.0;
        m_last_reported = false;
    }

    const size_t sampleRate = newSpectrums[0].sample_rate;
    // Lazy intialization of the spectrogram matcher.
    if (m_matcher == nullptr || m_matcher->sampleRate() != sampleRate){
        m_console.log("Loading spectrogram...");
        m_matcher = build_spectrogram_matcher(sampleRate);
    }

    // Feed spectrum one by one to the matcher:
    // newSpectrums are ordered from newest (largest timestamp) to oldest (smallest timestamp).
    // To feed the spectrum from old to new, we need to go through the vector in the reverse order:


//#define PA_DEBUG_FORCE_PLA_SOUND

#ifdef PA_DEBUG_FORCE_PLA_SOUND
    static int debug_count = 0;
    debug_count++;
    cout << "debug_count = " << debug_count << endl;
#endif
    
    bool found = false;
    const float threshold = get_score_threshold();
    for(auto it = newSpectrums.rbegin(); it != newSpectrums.rend(); it++){
        std::vector<AudioSpectrum> singleSpectrum = {*it};
        const float matcherScore = m_matcher->match(singleSpectrum);
        // std::cout << "error: " << matcherScore << std::endl;

        if (matcherScore == FLT_MAX){
            continue; // error or not enough spectrum history
        }

        // Record the lowest error found during the run
        m_lowest_error = std::min(m_lowest_error, matcherScore);

        found = matcherScore <= threshold;

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
            m_last_error = std::min(m_last_error, matcherScore);

            std::ostringstream os;
            os << m_audio_name << " found, score " << matcherScore << "/" << threshold << ", scale: " << m_matcher->lastMatchedScale();
            m_console.log(os.str(), COLOR_BLUE);
            audioFeed.add_overlay(curStamp+1-m_matcher->numMatchedWindows(), curStamp+1, m_detection_color);

            // Since the target audio is found, no need to check detection on the rest of the spectrums in `newSpectrums`.

            // Tell m_matcher to skip the remaining spectrums so that if `process_spectrums()` gets
            // called again on a newer batch of spectrums, m_matcher is happy.
            m_matcher->skip(std::vector<AudioSpectrum>(
                newSpectrums.begin(),
                newSpectrums.begin() + std::distance(it + 1, newSpectrums.rend())
            ));

            // Skip the remaining spectrums.
            break;
        }
    }

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
    if (m_on_shiny_callback == nullptr){
        return false;
    }

    m_last_reported = true;
    return m_on_shiny_callback(m_last_error);
}

void AudioPerSpectrumDetectorBase::clear(){
    m_matcher->clear();
}






}
