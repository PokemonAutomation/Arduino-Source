/*  Test Program Switch Audio
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Inference/InferenceException.h"
#include "CommonFramework/Tools/AudioFeed.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "TestProgramSwitchAudio.h"
#include "CommonFramework/AudioPipeline/AudioTemplate.h"

#include <set>
#include <iostream>
#include <list>
#include <thread>
#include <chrono>
#include <fstream>
#include <cfloat>
#include <map>
#include <sstream>
#include <array>

namespace PokemonAutomation{
namespace NintendoSwitch{


TestProgramAudio_Descriptor::TestProgramAudio_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "NintendoSwitch:TestProgramAudio",
        "Nintendo Switch", "Test Program Audio",
        "",
        "Test Program (Switch Audio)",
        FeedbackType::REQUIRED, true,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


TestProgramAudio::TestProgramAudio(const TestProgramAudio_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
}



// TODO: need to move this class to an audio inference folder:
class SpectrogramMatcher{
public:
    enum class Mode{
        NO_CONV,
        SPIKE_CONV,
    };

    SpectrogramMatcher(const QString& templateFilename, Mode mode);

    // Newer (larger timestamp) spectrums at beginning of `newSpectrums` while older (smaller
    // timestamp) spectrums at the end.
    // Return a match score.
    // In invalid cases (internal error or not enough windows), return FLT_MAX
    float match(const std::vector<std::shared_ptr<AudioSpectrum>>& newSpectrums);

    size_t numTemplateWindows() const { return m_template.numWindows(); }

    // Return latest timestamp from the stored audio spectrum stream.
    // Return SIZE_MAX if there is no spectrum yet.
    size_t latestTimestamp() const;

private:
    void conv(const float* src, size_t num, float* dst);
    float templateNorm() const;

private:
    AudioTemplate m_template;
    size_t m_numOriginalFrequencies = 0;
    size_t m_originalFreqStart = 0;
    size_t m_originalFreqEnd = 0;

    float m_templateNorm = 0.0f;
    size_t m_freqStart = 0;
    size_t m_freqEnd = 0;

    Mode m_mode = Mode::NO_CONV;

    const std::array<float, 18> m_spikeKernel = {-4.f, -3.f, -2.f, -1.f, 0.f, 1.f, 2.f, 3.f, 4.f, 4.f, 3.f, 2.f, 1.f, 0.f, -1.f, -2.f, -3.f, -4.f};

    std::list<std::shared_ptr<AudioSpectrum>> m_spectrums;
    std::list<float> m_spectrumNormSqrs;

    size_t m_lastStampTested = SIZE_MAX;
};


SpectrogramMatcher::SpectrogramMatcher(const QString& templateFilename, Mode mode) : m_mode(mode) {
    m_template = loadAudioTemplate(templateFilename);
    const size_t numTemplateWindows = m_template.numWindows();
    m_numOriginalFrequencies = m_template.numFrequencies();
    if (m_template.numFrequencies() == 0){  // Error case, failed to load template
        return;
    }

    // TODO: get sample rate as part of the AudioFeed interface:
    const int sampleRate = 48000;
    const int halfSampleRate = sampleRate / 2;

     // The frquency range from [0.0, (numTemplateFrequencies-1)/numTemplateFrequencies * halfSampleRate]
    // Since human can only hear as high as 20KHz sound, matching on frequencies >= 20KHz is meaningless.
    // So the index i of the max frequency we should be matching is the one with
    // i /numTemplateFrequencies * halfSampleRate <= 20KHz
    // i <= numTemplateFrequencies * 20K / halfSampleRate

    m_originalFreqStart = 5;
    m_originalFreqEnd = 20000 * m_numOriginalFrequencies / halfSampleRate + 1;
    
    if (m_mode == Mode::SPIKE_CONV){
        // Do convolution on audio template
        const size_t numConvedFrequencies = (m_originalFreqEnd - m_originalFreqStart) - m_spikeKernel.size() + 1;
        std::vector<float> temporaryBuffer(numConvedFrequencies * numTemplateWindows);
        for(size_t i = 0; i < numTemplateWindows; i++){
            conv(m_template.getWindow(i) + m_originalFreqStart, m_originalFreqEnd - m_originalFreqStart,
                temporaryBuffer.data() + i * numConvedFrequencies);
        }

        m_template = AudioTemplate(std::move(temporaryBuffer), numTemplateWindows);
        m_freqStart = 0;
        m_freqEnd = numConvedFrequencies;
    } else{
        m_freqStart = m_originalFreqStart;
        m_freqEnd = m_originalFreqEnd;
    }
    
    m_templateNorm = templateNorm();
}

size_t SpectrogramMatcher::latestTimestamp() const{
    if (m_spectrums.size() == 0){
        return SIZE_MAX;
    }
    return m_spectrums.front()->stamp;
}

void SpectrogramMatcher::conv(const float* src, size_t num, float* dst){
    if (num < m_spikeKernel.size()){
        return;
    }
    for(size_t i = 0; i < num-m_spikeKernel.size()+1; i++){
        dst[i] = 0.0f;
        for(size_t j = 0; j < m_spikeKernel.size(); j++){
            dst[i] += src[i+j] * m_spikeKernel[j];
        }
    }
}

float SpectrogramMatcher::templateNorm() const{
    float sumSqr = 0.0f;
    for(size_t i = 0; i < m_template.numWindows(); i++)
        for(size_t j = m_freqStart; j < m_freqEnd; j++){
            const float v = m_template.getWindow(i)[j];
            sumSqr += v * v;
        }
    return std::sqrt(sumSqr);
}

float SpectrogramMatcher::match(const std::vector<std::shared_ptr<AudioSpectrum>>& newSpectrums){
    for(auto it = newSpectrums.rbegin(); it != newSpectrums.rend(); it++){
        auto spectrum = *it;
        if (m_numOriginalFrequencies != spectrum->magnitudes.size()){
            std::cout << "Error: number of frequencies don't match in SpectrogramMatcher::match() " << 
                m_numOriginalFrequencies << " " << spectrum->magnitudes.size() << std::endl;
            return FLT_MAX;
        }

        if (m_mode == Mode::SPIKE_CONV){
            // Do the conv on new spectrum too.
            std::vector<float> convedSpectrum(m_template.numFrequencies());
            conv(spectrum->magnitudes.data() + m_originalFreqStart, 
                m_originalFreqEnd - m_originalFreqStart, convedSpectrum.data());
            
            spectrum = std::make_shared<AudioSpectrum>(spectrum->stamp, std::move(convedSpectrum));
        }

        // Compute the norm square (= sum squares) of the spectrum, used for matching:
        float spectrumNormSqr = 0.0f;
        for(size_t i = m_freqStart; i < m_freqEnd; i++){
            spectrumNormSqr += spectrum->magnitudes[i] * spectrum->magnitudes[i];
        }
        m_spectrumNormSqrs.push_front(spectrumNormSqr);

        m_spectrums.emplace_front(std::move(spectrum));
    }
  
    while(m_spectrums.size() > m_template.numWindows()){
        m_spectrums.pop_back();
        m_spectrumNormSqrs.pop_back();
    }

    if (m_spectrums.size() < m_template.numWindows()){
        return FLT_MAX;
    }

    // Check whether the stored spectrums' timestamps are continuous:
    size_t curStamp = m_spectrums.front()->stamp;
    size_t lastStamp = curStamp + 1;
    for(const auto& s : m_spectrums){
        if (s->stamp != lastStamp-1){
            std::cout << "Error: SpectrogramMatcher's spectrum timestamps are not continuous:" << std::endl;

            for(const auto& sp : m_spectrums){
                std::cout << sp->stamp << ", ";
            }
            std::cout << std::endl;
            return FLT_MAX;
        }
        lastStamp--;
    }

    if (m_lastStampTested != SIZE_MAX && curStamp <= m_lastStampTested){
        return FLT_MAX;
    }
    m_lastStampTested = curStamp;
    
    // Do the match:
    auto iter = m_spectrums.begin();
    auto iter2 = m_spectrumNormSqrs.begin();
    float streamSumSqr = 0.0f;
    float sumMulti = 0.0f;
    for(size_t i = 0; i < m_template.numWindows(); i++, iter++, iter2++){
        // match in order from latest window to oldest
        const float* templateData = m_template.getWindow(m_template.numWindows()-1-i);
        const float* streamData = (*iter)->magnitudes.data();
        streamSumSqr += *iter2;
        for(size_t j = m_freqStart; j < m_freqEnd; j++){
            sumMulti += templateData[j] * streamData[j];
        }
    }
    float scale = (streamSumSqr < 1e-6f ? 1.0f : sumMulti / streamSumSqr);
    // std::cout << curStamp << " Matcher " << scale << " " << sumMulti << "/" << streamSumSqr << std::endl;

    iter = m_spectrums.begin();
    iter2 = m_spectrumNormSqrs.begin();
    float sum = 0.0f;
    for(size_t i = 0; i < m_template.numWindows(); i++, iter++, iter2++){
        // match in order from latest window to oldest
        const float* templateData = m_template.getWindow(m_template.numWindows()-1-i);
        const float* streamData = (*iter)->magnitudes.data();
        for(size_t j = m_freqStart; j < m_freqEnd; j++){
            float d = templateData[j] - scale * streamData[j];
            sum += d * d;
        }
    }
    float distance = sqrt(sum) / m_templateNorm;

    // std::cout << "(" << curStamp+1-m_template.numWindows() << ", " <<  curStamp+1 << "): " << distance << std::endl;

    return distance;
}

void TestProgramAudio::program(SingleSwitchProgramEnvironment& env){
//    Stats& stats = env.stats<Stats>();
//    env.update_stats();

    //  Connect the controller.
    pbf_move_right_joystick(env.console, 0, 255, 10, 0);

    std::cout << "Running audio test program." << std::endl;

    QString shinyFilename = "./heracrossShinyTemplateCompact.wav";

    SpectrogramMatcher matcher(shinyFilename, SpectrogramMatcher::Mode::SPIKE_CONV);


    auto& audioFeed = env.console.audio();

    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        std::vector<std::shared_ptr<AudioSpectrum>> newSpectrums;
        if (matcher.latestTimestamp() == SIZE_MAX){
            audioFeed.spectrums_latest(matcher.numTemplateWindows(), newSpectrums);
        } else{
            audioFeed.spectrums_since(matcher.latestTimestamp() + 1, newSpectrums);
        }

        for(auto it = newSpectrums.rbegin(); it != newSpectrums.rend(); it++){
            // feed spectrum one by one to the matcher:
            std::vector<std::shared_ptr<AudioSpectrum>> singleSpectrum = {*it};
            float matcherScore = matcher.match(singleSpectrum);

            if (matcherScore == FLT_MAX){
                continue;
            }

            bool found = matcherScore <= 0.7;

            size_t curStamp = matcher.latestTimestamp();
            std::cout << "(" << curStamp+1-matcher.numTemplateWindows() << ", " <<  curStamp+1 << "): " << matcherScore << 
                (found ? " FOUND!" : "") << std::endl;
    
            if (found) {
                audioFeed.add_overlay(curStamp+1-matcher.numTemplateWindows(), curStamp+1);
            }
        }
    }

    std::cout << "Audio test program finished." << std::endl;

//    env.update_stats();
    GO_HOME_WHEN_DONE.run_end_of_program(env.console);
}


void searchAudioDump(){

    QString shinyFilename = "./heracrossShinyTemplateCompact.wav";
    AudioTemplate shinyTemplate = loadAudioTemplate(shinyFilename);
    const size_t numTemplateWindows = shinyTemplate.numWindows();
    const size_t numTemplateFrequencies = shinyTemplate.numFrequencies();

    
    // TODO: get sample rate as part of the AudioFeed interface:
    const int sampleRate = 48000;
    const int halfSampleRate = sampleRate / 2;

    std::string fileListFile = "./scripts/short_audio_files.txt";
    // std::string fileListFile = "1.txt";
    // std::string fileListFile = "./scripts/all_audio_files.txt";
    std::ifstream fin(fileListFile.c_str());
    std::vector<std::string> fileList;
    while(!fin.eof()){
        std::string line;
        std::getline(fin, line);
        fileList.push_back(line);
        fin >> std::ws;
    }
    std::cout << "File num " << fileList.size() << std::endl;

    std::map<float, std::string> closestFiles;

    std::ofstream fout("file_check_output.txt");

    for(size_t fileIdx = 0; fileIdx < fileList.size(); fileIdx++){
        const auto& path = fileList[fileIdx];
        std::ostringstream os;
        os << "File " << fileIdx << "/" << fileList.size() << " " << path << " ";
        AudioTemplate audio = loadAudioTemplate(QString(path.c_str()));
        if (audio.numWindows() == 0){
            os << "Fail" << std::endl;
            fout << os.str();
            std::cout << os.str() << std::flush;
            continue;
        }

        // audio.scale(2.0);

        os << "#W " << audio.numWindows() << " ";

        // match!
        float minDistance = FLT_MAX;
        for(size_t audioIdx = 0; audioIdx < audio.numWindows(); audioIdx++){
            float streamSumSqr = 0.0f;
            float sumMulti = 0.0f;
            for(size_t i = 0; i < numTemplateWindows; i++){
                // match in order from latest window to oldest
                const float* templateData = shinyTemplate.getWindow(i);
                if (audioIdx + i < audio.numWindows()){
                    const float* streamData = audio.getWindow(audioIdx+i);
                    for(size_t j = 1; j < numTemplateFrequencies; j++){
                        streamSumSqr += streamData[j] * streamData[j];
                        sumMulti += templateData[j] * streamData[j];
                    }
                }
            }
            float scale = (streamSumSqr < 1e-6f ? 1.0f : sumMulti / streamSumSqr);

            float sum = 0.0f;
            for(size_t i = 0; i < numTemplateWindows; i++){
                // match in order from latest window to oldest
                const float* templateData = shinyTemplate.getWindow(i);
                if (audioIdx + i < audio.numWindows()){
                    const float* streamData = audio.getWindow(audioIdx+i);
                    for(size_t j = 1; j < numTemplateFrequencies; j++){
                        float d = templateData[j] - scale * streamData[j];
                        sum += d * d;
                    }
                }else{
                    for(size_t j = 1; j < numTemplateFrequencies; j++){
                        sum += templateData[j] * templateData[j];
                    }
                }
            }
            float distance = sqrt(sum);

            minDistance = std::min(minDistance, distance);
        } // end audio Idx

        os << "dist " << minDistance << std::endl;
        fout << os.str();
        std::cout << os.str() << std::flush;

        closestFiles.emplace(minDistance, path);
    }

    fout.close();

    auto it = closestFiles.begin();
    std::cout << "--------------" << std::endl;
    fout.open("file_check_output_sorted.txt");
    for(int i = 0; it != closestFiles.end(); i++, it++){
        if (i < 40)
            std::cout << it->first << ", " << it->second << std::endl;
        fout << it->first << ", " << it->second << std::endl;
    }
    fout.close();
    return;
}


}
}
