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
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_AlphaMusicDetector.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_AlphaRoarDetector.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ItemDropSoundDetector.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLA_SoundListener.h"

#include <QMessageBox>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;



SoundListener_Descriptor::SoundListener_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:SoundListener",
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
            {SoundType::Shiny,      "shiny",        "Shiny Sound"},
            {SoundType::AlphaRoar,  "alpha-roar",   "Alpha Roar"},
            {SoundType::AlphaMusic, "alpha-music",  "Alpha Music"},
            {SoundType::ItemDrop,   "item-drop",    "Item Drop Sound"},
        },
        LockWhileRunning::LOCKED,
        SoundType::Shiny
    )
    , STOP_ON_DETECTED_SOUND(
        "<b>Stop on the detected sound</b><br>Stop program when the sound is detected.",
        LockWhileRunning::LOCKED,
        false
    )
{
    PA_ADD_OPTION(SOUND_TYPE);
    PA_ADD_OPTION(STOP_ON_DETECTED_SOUND);
}


// void searchAlphaRoarFromAudioDump();

void SoundListener::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    //  Connect the controller.
    // pbf_move_right_joystick(context, 0, 255, 10, 0);

    // searchAlphaRoarFromAudioDump();
    // return;

    std::cout << "Running audio test program." << std::endl;

    std::unique_ptr<AudioInferenceCallback> detector;
    auto action = [&](float error_coefficient) -> bool{
        // This lambda function will be called when the sound is detected.
        // Its return will determine whether to stop the program:
        return STOP_ON_DETECTED_SOUND;
    };

    SoundType type = SOUND_TYPE;
    switch (type){
    case SoundType::Shiny:
        detector = std::make_unique<ShinySoundDetector>(env.console.logger(), env.console, action);
        break;
    case SoundType::AlphaRoar:
        detector = std::make_unique<AlphaRoarDetector>(env.console.logger(), env.console, action);
        break;
    case SoundType::AlphaMusic:
        detector = std::make_unique<AlphaMusicDetector>(env.console.logger(), env.console, action);
        break;
    case SoundType::ItemDrop:
        detector = std::make_unique<ItemDropSoundDetector>(env.console.logger(), env.console, action);
        break;
    default:
        throw InternalProgramError(
            &env.logger(), PA_CURRENT_FUNCTION,
            "Not such sound detector as sound type " + std::to_string((size_t)type)
        );
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
