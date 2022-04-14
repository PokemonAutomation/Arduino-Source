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


ShinySoundDetector::ShinySoundDetector(ConsoleHandle& console, bool stop_on_detected)
    // Use a green as the detection color because the shiny symbol in LA is green.
    : AudioPerSpectrumDetectorBase("ShinySoundDetector", "Shiny sound", COLOR_GREEN, console, stop_on_detected)
{}

ShinySoundResults ShinySoundDetector::results(){
    ShinySoundResults results;
    SpinLockGuard lg(m_lock);
    results.screenshot = m_screenshot;
    results.error_coefficient = m_error_coefficient;
    return results;
}


float ShinySoundDetector::get_score_threshold() const{
    return (float)GameSettings::instance().SHINY_SHOUND_THRESHOLD2;
}

std::unique_ptr<SpectrogramMatcher> ShinySoundDetector::build_spectrogram_matcher(size_t sampleRate){
    return std::make_unique<SpectrogramMatcher>(
        AudioTemplateCache::instance().get_throw("PokemonLA/ShinySound", sampleRate),
        SpectrogramMatcher::Mode::SPIKE_CONV, sampleRate,
        GameSettings::instance().SHINY_SHOUND_LOW_FREQUENCY
    );
}








ShinySoundDetector2::~ShinySoundDetector2(){
    try{
        log_results();
    }catch (...){}
}
ShinySoundDetector2::ShinySoundDetector2(ConsoleHandle& console, std::function<bool(float error_coefficient)> on_shiny_callback)
    : AudioInferenceCallback("ShinySoundDetector")
    , m_console(console)
    , m_on_shiny_callback(std::move(on_shiny_callback))
    , m_lowest_error(1.0)
    , m_last_timestamp(WallClock::min())
    , m_last_error(1.0)
    , m_last_reported(false)
{}
void ShinySoundDetector2::log_results(){
    std::stringstream ss;
    ss << m_lowest_error;
    if (m_last_timestamp != WallClock::min()){
        m_console.log("Shiny detected! Error Coefficient = " + ss.str(), COLOR_BLUE);
    }else{
        m_console.log("No shiny detected. Error Coefficient = " + ss.str(), COLOR_PURPLE);
    }
}


bool ShinySoundDetector2::process_spectrums(
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
    bool found = false;
    for (auto it = newSpectrums.rbegin(); it != newSpectrums.rend(); it++){
        std::vector<AudioSpectrum> singleSpectrum = {*it};
        float matcherScore = m_matcher->match(singleSpectrum);

        if (matcherScore == FLT_MAX){
            continue; // error or not enough spectrum history
        }

        const float threshold = (float)GameSettings::instance().SHINY_SHOUND_THRESHOLD2;
        found = matcherScore <= threshold;
//        cout << matcherScore << endl;

        m_lowest_error = std::min(m_lowest_error, matcherScore);

        size_t curStamp = m_matcher->latestTimestamp();
        // std::cout << "(" << curStamp+1-m_matcher->numTemplateWindows() << ", " <<  curStamp+1 << "): " << matcherScore <<
        //     (found ? " FOUND!" : "") << std::endl;

        if (found){
            if (m_last_error >= 1.0){
                m_last_timestamp = now;
            }
            m_last_error = std::min(m_last_error, matcherScore);

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

            break;
        }
    }

    //  No shiny detected.
    if (!found){
        return false;
    }

    //  Shiny detected, but haven't waited long enough to measure its magnitude.
    if (m_last_timestamp + std::chrono::milliseconds(200) > now){
        return false;
    }

    //  Already reported this shiny. Don't report again.
    if (m_last_reported){
        return false;
    }

    //  No callback. Can't report.
    if (m_on_shiny_callback == nullptr){
        return false;
    }

    bool ret = m_on_shiny_callback(m_last_error);
    m_last_reported = true;
    return ret;
}




}
}
}
