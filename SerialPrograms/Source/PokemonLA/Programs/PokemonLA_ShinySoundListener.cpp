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
#include "PokemonLA_ShinySoundListener.h"
#include "PokemonLA/Inference/PokemonLA_ShinySoundDetector.h"
#include "CommonFramework/AudioPipeline/AudioTemplate.h"
#include "CommonFramework/Inference/AudioInferenceSession.h"

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
namespace PokemonLA{

ShinySoundListener_Descriptor::ShinySoundListener_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:ShinySoundListener",
        STRING_POKEMON + " LA", "Shiny Sound Listener",
        "",
        "Detect shiny sound from audio stream.",
        FeedbackType::REQUIRED, true,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


ShinySoundListener::ShinySoundListener(const ShinySoundListener_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
{}



void ShinySoundListener::program(SingleSwitchProgramEnvironment& env){
    //  Connect the controller.
    // pbf_move_right_joystick(env.console, 0, 255, 10, 0);

    std::cout << "Running audio test program." << std::endl;

    auto& audioFeed = env.console.audio();
    
    ShinySoundDetector detector(env.console);

#if 0
    AsyncAudioInferenceSession session(env, env.console, audioFeed);
    session += detector;

    pbf_wait(env.console, 125 * 30);
    env.wait_for(std::chrono::milliseconds(30000));

    if (session.stop()){
        std::cout << "Found shiny!" << std::endl;
    }
#endif

    size_t lastTimestamp = SIZE_MAX;
    // Stores new spectrums from audio feed. The newest spectrum (with largest timestamp) is at
    // the front of the vector.
    std::vector<std::shared_ptr<PokemonAutomation::AudioSpectrum>> spectrums;

    while(true){
        env.wait_for(std::chrono::milliseconds(10));
        env.check_stopping();
        
        spectrums.clear();
        if (lastTimestamp == SIZE_MAX){
            audioFeed.spectrums_latest(1, spectrums);
        } else{
            // Note: in this file we never consider the case that stamp may overflow.
            // It requires on the order of 1e10 years to overflow if we have about 25ms per stamp.
            audioFeed.spectrums_since(lastTimestamp+1, spectrums);
        }
        if (spectrums.size() > 0){
            // spectrums[0] has the newest spectrum with the largest stamp:
            lastTimestamp = spectrums[0]->stamp;
        }

        bool found = detector.process_spectrums(spectrums, audioFeed);

        if (found){
            std::cout << "FOUND" << std::endl;
        }
    }

    std::cout << "Audio test program finished." << std::endl;
}

// A function used to search for the shiny sound on LA audio dump.
// But we didn't find the shound sound :P
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
}
