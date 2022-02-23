/*  Shiny Sound Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/AudioFeed.h"
#include "PokemonLA_ShinySoundDetector.h"

#include <QString>

#include <cfloat>
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



ShinySoundDetector::ShinySoundDetector(Logger& logger)
    : AudioInferenceCallback("ShinySoundDetector")
    , m_logger(logger)
{
    // TODO : handle case with sample rate mismatch between file and capture card:

    // TODO: move the template file to the resource folder
    QString shinyFilename = "./heracrossShinyTemplateCompact.wav";

    m_matcher = std::make_unique<SpectrogramMatcher>(shinyFilename, SpectrogramMatcher::Mode::SPIKE_CONV);
}


bool ShinySoundDetector::process_spectrums(
        const std::vector<std::shared_ptr<AudioSpectrum>>& newSpectrums,
        AudioFeed& audioFeed
){
    // Feed spectrum one by one to the matcher:
    // newSpectrums are ordered from newest (largest timestamp) to oldest (smallest timestamp).
    // To feed the spectrum from old to new, we need to go through the vector in the reverse order:
    
    for(auto it = newSpectrums.rbegin(); it != newSpectrums.rend(); it++){
        std::vector<std::shared_ptr<AudioSpectrum>> singleSpectrum = {*it};
        float matcherScore = m_matcher->match(singleSpectrum);

        if (matcherScore == FLT_MAX){
            continue; // error or not enough spectrum history
        }

        bool found = matcherScore <= 0.7;

        size_t curStamp = m_matcher->latestTimestamp();
        // std::cout << "(" << curStamp+1-m_matcher->numTemplateWindows() << ", " <<  curStamp+1 << "): " << matcherScore << 
        //     (found ? " FOUND!" : "") << std::endl;

        if (found) {
            m_logger.log("Shiny sound find", COLOR_RED);
            audioFeed.add_overlay(curStamp+1-m_matcher->numTemplateWindows(), curStamp+1, COLOR_RED);
            // Tell m_matcher to skip the remaining spectrums so that if `process_spectrums()` gets
            // called again on a newer batch of spectrums, m_matcher is happy.
            m_matcher->skip(std::vector<std::shared_ptr<AudioSpectrum>>(newSpectrums.begin(), 
                newSpectrums.begin() + std::distance(it+1, newSpectrums.rend())));
            return true;
        }
    }

    return false;
}





}
}
}
