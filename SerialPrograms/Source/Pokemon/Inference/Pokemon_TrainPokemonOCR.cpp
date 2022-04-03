/*  Train Pokemon Name OCR
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QDirIterator>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/ParallelTaskRunner.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "CommonFramework/OCR/OCR_Filtering.h"
#include "CommonFramework/OCR/OCR_LargeDictionaryMatcher.h"
#include "CommonFramework/OCR/OCR_TrainingTools.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "Pokemon_TrainPokemonOCR.h"

namespace PokemonAutomation{
namespace Pokemon{


TrainPokemonOCR_Descriptor::TrainPokemonOCR_Descriptor()
    : RunnableComputerProgramDescriptor(
        "PokemonSwSh:TrainPokemonNameOCR",
        STRING_POKEMON, "Train " + STRING_POKEMON + " Name OCR",
        "",
        "Train " + STRING_POKEMON + " Name OCR"
    )
{}



TrainPokemonOCR::TrainPokemonOCR(const TrainPokemonOCR_Descriptor& descriptor)
    : RunnableComputerProgramInstance(descriptor)
    , DIRECTORY(
        false,
        "<b>Training Data Directory:</b> (Relative to \"TrainingData/\")",
        "PokemonNameOCR/",
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
    PA_ADD_OPTION(DIRECTORY);
    PA_ADD_OPTION(MODE);
    PA_ADD_OPTION(THREADS);
}


void TrainPokemonOCR::program(ProgramEnvironment& env, CancellableScope& scope){
    OCR::TrainingSession session(env.logger(), scope, DIRECTORY);
    session.generate_large_dictionary(
        "Pokemon/PokemonNameOCR/",
        "PokemonOCR-",
        MODE != 0,
        THREADS
    );
}


}
}

