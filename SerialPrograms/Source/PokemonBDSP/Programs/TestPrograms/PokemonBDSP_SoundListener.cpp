/*  Sound Listener
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
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "CommonFramework/AudioPipeline/AudioFeed.h"
#include "CommonFramework/AudioPipeline/AudioTemplate.h"
#include "CommonFramework/InferenceInfra/InferenceSession.h"
#include "CommonFramework/Inference/AudioTemplateCache.h"
#include "CommonFramework/Inference/SpectrogramMatcher.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Inference/Sounds/PokemonBDSP_ShinySoundDetector.h"
#include "PokemonBDSP_SoundListener.h"

#include <QMessageBox>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
    using namespace Pokemon;


SoundListener_Descriptor::SoundListener_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:SoundListener",
        STRING_POKEMON + " LA", "Sound Listener",
        "",
        "Test sound detectors listening to audio stream.",
        FeedbackType::REQUIRED, AllowCommandsWhenRunning::ENABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


SoundListener::SoundListener()
    : SOUND_TYPE("<b>Which Sound to Detect</b>",
        {
            {SoundType::SHINY, "shiny", "Shiny Sound"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        SoundType::SHINY
    )
    , STOP_ON_DETECTED_SOUND(
        "<b>Stop on the detected sound</b><br>Stop program when the sound is detected.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
{
    PA_ADD_OPTION(SOUND_TYPE);
    PA_ADD_OPTION(STOP_ON_DETECTED_SOUND);
}


// void search_alpha_roar_from_audio_dump();

void SoundListener::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    // search_alpha_roar_from_audio_dump();
    // return;

    std::cout << "Running audio test program." << std::endl;

    std::shared_ptr<AudioInferenceCallback> detector;
    auto action = [&](float error_coefficient) -> bool{
        // This lambda function will be called when the sound is detected.
        // Its return will determine whether to stop the program:
        return STOP_ON_DETECTED_SOUND;
    };

    SoundType type = SOUND_TYPE;
    switch (type){
    case SoundType::SHINY:
        detector = std::make_shared<ShinySoundDetector>(env.console, action);
        break;
    default:
        throw InternalProgramError(
            &env.logger(), PA_CURRENT_FUNCTION,
            "Not such sound detector as sound type " + std::to_string((size_t)type)
        );
        return;
    }

    InferenceSession session(
        context, env.console,
        {{*detector, std::chrono::milliseconds(20)}}
    );
    context.wait_until_cancel();

    std::cout << "Audio test program Sound listener finished." << std::endl;
}


// A function used to search for the alpha roar on LA audio dump.
// But we didn't find the shound sound :P
void search_alpha_roar_from_audio_dump(){

    const size_t SAMPLE_RATE = 48000;

    SpectrogramMatcher matcher(
        "Alpha Roar",
        AudioTemplateCache::instance().get_throw("PokemonBDSP/AlphaRoar", SAMPLE_RATE),
        SpectrogramMatcher::Mode::RAW, SAMPLE_RATE,
        100.0
    );
    
    // std::string file_listFile = "./scripts/short_audio_files.txt";
    std::string file_listFile = "1.txt";
    // std::string file_listFile = "./scripts/all_audio_files.txt";
    std::ifstream fin(file_listFile.c_str());
    std::vector<std::string> file_list;
    while(!fin.eof()){
        std::string line;
        std::getline(fin, line);
        file_list.push_back(line);
        fin >> std::ws;
    }
    std::cout << "File num " << file_list.size() << std::endl;

    std::map<float, std::string> closest_files;

    std::ofstream fout("file_check_output.txt");

    for(size_t fileIdx = 0; fileIdx < file_list.size(); fileIdx++){
        matcher.clear();


        const auto& path = file_list[fileIdx];
        std::ostringstream os;
        os << "File " << fileIdx << "/" << file_list.size() << " " << path << " ";
        AudioTemplate audio = loadAudioTemplate(path);
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
        std::vector<AudioSpectrum> new_spectrums;
        size_t numStreamWindows = std::max(matcher.numMatchedWindows(), audio.numWindows());
        for(size_t audioIdx = 0; audioIdx < numStreamWindows; audioIdx++){
            new_spectrums.clear();
            AlignedVector<float> freqVector(audio.numFrequencies());
            if (audioIdx < audio.numWindows()){
                const float * freq = audio.getWindow(audioIdx);
                memcpy(freqVector.data(), freq, sizeof(float) * audio.numFrequencies());
            } else{
                // add zero-freq window
            }
            new_spectrums.emplace_back(
                audioIdx, SAMPLE_RATE,
                std::make_unique<AlignedVector<float>>(std::move(freqVector))
            );
            float score = matcher.match(new_spectrums);
            minScore = std::min(score, minScore);
        } // end audio Idx

        os << "dist " << minScore << std::endl;
        fout << os.str();
        std::cout << os.str() << std::flush;

        closest_files.emplace(minScore, path);
    }

    fout.close();

    auto it = closest_files.begin();
    std::cout << "--------------" << std::endl;
    fout.open("file_check_output_sorted.txt");
    for(int i = 0; it != closest_files.end(); i++, it++){
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
