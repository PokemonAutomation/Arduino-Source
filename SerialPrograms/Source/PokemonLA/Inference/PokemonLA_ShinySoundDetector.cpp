/*  Shiny Sound Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QString>
#include "CommonFramework/Inference/SpectrogramMatcher.h"
#include "CommonFramework/Inference/AudioTemplateCache.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/AudioFeed.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA_ShinySoundDetector.h"

#include <sstream>
#include <cfloat>
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



ShinySoundDetector::~ShinySoundDetector(){
    log_results();
}
ShinySoundDetector::ShinySoundDetector(ConsoleHandle& console, bool stop_on_detected)
    : AudioInferenceCallback("ShinySoundDetector")
    , m_console(console)
    , m_stop_on_detected(stop_on_detected)
    , m_detected(false)
    , m_time_detected(std::chrono::system_clock::time_point::min())
    , m_error_coefficient(1.0)
{}
void ShinySoundDetector::log_results(){
    std::stringstream ss;
    ss << m_error_coefficient;
    if (detected()){
        m_console.log("Shiny detected! Error Coefficient = " + ss.str(), COLOR_BLUE);
    }else{
        m_console.log("No shiny detected. Error Coefficient = " + ss.str(), COLOR_PURPLE);
    }
}
ShinySoundResults ShinySoundDetector::results(){
    ShinySoundResults results;
    SpinLockGuard lg(m_lock);
    results.screenshot = m_screenshot;
    results.error_coefficient = m_error_coefficient;
    return results;
}


bool ShinySoundDetector::process_spectrums(
    const std::vector<AudioSpectrum>& newSpectrums,
    AudioFeed& audioFeed
){
    if (newSpectrums.empty()){
        return false;
    }

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    size_t sampleRate = newSpectrums[0].sample_rate;
    if (m_matcher == nullptr || m_matcher->sampleRate() != sampleRate){
        m_console.log("Loading spectrogram...");
        m_matcher = std::make_unique<SpectrogramMatcher>(
            AudioTemplateCache::instance().get_throw("PokemonLA/ShinySound", sampleRate),
            SpectrogramMatcher::Mode::SPIKE_CONV, sampleRate,
            GameSettings::instance().SHINY_SHOUND_LOW_FREQUENCY
        );
    }

    // Feed spectrum one by one to the matcher:
    // newSpectrums are ordered from newest (largest timestamp) to oldest (smallest timestamp).
    // To feed the spectrum from old to new, we need to go through the vector in the reverse order:
    
//    static int c = 0;
//    c++;
    for(auto it = newSpectrums.rbegin(); it != newSpectrums.rend(); it++){
        std::vector<AudioSpectrum> singleSpectrum = {*it};
        float matcherScore = m_matcher->match(singleSpectrum);

        if (matcherScore == FLT_MAX){
            continue; // error or not enough spectrum history
        }

        const float threshold = (float)GameSettings::instance().SHINY_SHOUND_THRESHOLD2;
        bool found = matcherScore <= threshold;
//        cout << matcherScore << endl;

        m_error_coefficient = std::min(m_error_coefficient, matcherScore);

        size_t curStamp = m_matcher->latestTimestamp();
        // std::cout << "(" << curStamp+1-m_matcher->numTemplateWindows() << ", " <<  curStamp+1 << "): " << matcherScore << 
        //     (found ? " FOUND!" : "") << std::endl;

        if (found){
            {
                SpinLockGuard lg(m_lock);
                m_screenshot = m_console.video().snapshot();
            }
            std::ostringstream os;
            os << "Shiny sound find, score " << matcherScore << "/" << threshold << ", scale: " << m_matcher->lastMatchedScale();
            m_console.log(os.str(), COLOR_BLUE);
            audioFeed.add_overlay(curStamp+1-m_matcher->numTemplateWindows(), curStamp+1, COLOR_RED);
            // Tell m_matcher to skip the remaining spectrums so that if `process_spectrums()` gets
            // called again on a newer batch of spectrums, m_matcher is happy.
            m_matcher->skip(std::vector<AudioSpectrum>(
                newSpectrums.begin(),
                newSpectrums.begin() + std::distance(it + 1, newSpectrums.rend())
            ));

            if (!m_detected.load(std::memory_order_acquire)){
                m_time_detected = now;
                m_detected.store(true, std::memory_order_release);
            }

            break;
        }
    }

    if (!m_stop_on_detected){
        return false;
    }
    if (!m_detected.load(std::memory_order_acquire)){
        return false;
    }

    //  Don't return true for another 200ms so we can get a measurement of how strong this detection is.
    return m_time_detected + std::chrono::milliseconds(200) <= now;
}

void ShinySoundDetector::clear(){
    m_matcher->clear();
}



}
}
}
