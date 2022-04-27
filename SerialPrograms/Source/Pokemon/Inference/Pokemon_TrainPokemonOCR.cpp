/*  Train Pokemon Name OCR
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/ParallelTaskRunner.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/OCR/OCR_TrainingTools.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "Pokemon_NameReader.h"
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
        THREADS,
        {
            {0xff000000, 0xff404040},
            {0xff000000, 0xff606060},
            {0xff000000, 0xff808080},
            {0xff808080, 0xffffffff},
            {0xffa0a0a0, 0xffffffff},
        },
        PokemonNameReader::MAX_LOG10P, PokemonNameReader::MAX_LOG10P_SPREAD
    );
}


}
}

