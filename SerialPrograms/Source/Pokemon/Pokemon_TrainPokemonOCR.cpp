/*  Train Pokemon Name OCR
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QDirIterator>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/ParallelTaskRunner.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/OCR/RawOCR.h"
#include "CommonFramework/OCR/Filtering.h"
#include "CommonFramework/OCR/LargeDictionaryMatcher.h"
#include "CommonFramework/OCR/TrainingTools.h"
#include "Pokemon_TrainPokemonOCR.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Pokemon{


TrainPokemonOCR_Descriptor::TrainPokemonOCR_Descriptor()
    : RunnableComputerProgramDescriptor(
        "PokemonSwSh:TrainPokemonNameOCR",
        "Train " + STRING_POKEMON + " Name OCR",
        "",
        "Train " + STRING_POKEMON + " Name OCR"
    )
{}



TrainPokemonOCR::TrainPokemonOCR(const TrainPokemonOCR_Descriptor& descriptor)
    : RunnableComputerProgramInstance(descriptor)
    , DIRECTORY(
        "<b>Training Data Directory:</b> (Relative to \"TrainingData/\")",
        "PokemonNameOCR/"
    )
    , MODE(
        "<b>Mode:</b>",
        {
            "Start Fresh: Use only baseline strings. (1st candidate of each entry in above path)",
            "Incremental: Build off of the existing training data in the above path.",
        },
        0
    )
    , THREADS(
        "<b>Worker Threads:</b>",
        std::thread::hardware_concurrency()
    )
{
    m_options.emplace_back(&DIRECTORY, "DIRECTORY");
    m_options.emplace_back(&MODE, "MODE");
    m_options.emplace_back(&THREADS, "THREADS");
}


void TrainPokemonOCR::program(ProgramEnvironment& env){
    OCR::TrainingSession session(env, DIRECTORY);
    session.generate_large_dictionary(
        env,
        "Pokemon/PokemonNameOCR/",
        "PokemonOCR-",
        MODE != 0,
        THREADS
    );
}


}
}

