/*  Alpha Roar Listener
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <float.h>
#include <chrono>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include "CommonFramework/AudioPipeline/AudioFeed.h"
#include "CommonFramework/AudioPipeline/AudioTemplate.h"
#include "CommonFramework/InferenceInfra/InferenceSession.h"
#include "CommonFramework/Inference/AudioTemplateCache.h"
#include "CommonFramework/Inference/SpectrogramMatcher.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_AlphaRoarDetector.h"
#include "PokemonLA_AlphaRoarListener.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

AlphaRoarListener_Descriptor::AlphaRoarListener_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:AlphaRoarListener",
        STRING_POKEMON + " LA", "Alpha Roar Listener",
        "",
        "Detect alpha roar from audio stream.",
        FeedbackType::REQUIRED, true,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


AlphaRoarListener::AlphaRoarListener(const AlphaRoarListener_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , STOP_ON_ALPHA_ROAR("<b>Stop on Alpha Roar</b><br>Stop program when an alpha roar is heard.", false)
{
    PA_ADD_OPTION(STOP_ON_ALPHA_ROAR);
}


void searchAlphaRoarFromAudioDump();

void AlphaRoarListener::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    //  Connect the controller.
    // pbf_move_right_joystick(context, 0, 255, 10, 0);

    // searchAlphaRoarFromAudioDump();
    // return;

    std::cout << "Running audio test program." << std::endl;
    
    AlphaRoarDetector detector(env.console, [&](float error_coefficient) -> bool{
        // This lambda function will be called when an alpha roar is detected.
        // Its return will determine whether to stop the program:
        return STOP_ON_ALPHA_ROAR;
    });

    InferenceSession session(
        context, env.console,
        {{detector, std::chrono::milliseconds(20)}}
    );
    context.wait_until_cancel();


    std::cout << "Audio test program finished." << std::endl;
}

// A function used to search for the alpha roar on LA audio dump.
// But we didn't find the shound sound :P
void searchAlphaRoarFromAudioDump(){

    const size_t SAMPLE_RATE = 48000;

    SpectrogramMatcher matcher(
        AudioTemplateCache::instance().get_throw("PokemonLA/AlphaRoar", SAMPLE_RATE),
        SpectrogramMatcher::Mode::RAW, SAMPLE_RATE,
        100.0
    );
    
    // std::string fileListFile = "./scripts/short_audio_files.txt";
    std::string fileListFile = "1.txt";
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
        matcher.clear();


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
        float minScore = FLT_MAX;
        std::vector<AudioSpectrum> newSpectrums;
        size_t numStreamWindows = std::max(matcher.numMatchedWindows(), audio.numWindows());
        for(size_t audioIdx = 0; audioIdx < numStreamWindows; audioIdx++){
            newSpectrums.clear();
            AlignedVector<float> freqVector(audio.numFrequencies());
            if (audioIdx < audio.numWindows()){
                const float * freq = audio.getWindow(audioIdx);
                memcpy(freqVector.data(), freq, sizeof(float) * audio.numFrequencies());
            } else{
                // add zero-freq window
            }
            newSpectrums.emplace_back(
                audioIdx, SAMPLE_RATE,
                std::make_unique<AlignedVector<float>>(std::move(freqVector))
            );
            float score = matcher.match(newSpectrums);
            minScore = std::min(score, minScore);
        } // end audio Idx

        os << "dist " << minScore << std::endl;
        fout << os.str();
        std::cout << os.str() << std::flush;

        closestFiles.emplace(minScore, path);
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
