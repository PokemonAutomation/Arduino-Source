/*  Train Pokemon Name OCR
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Concurrency/ParallelTaskRunner.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/OCR/OCR_TrainingTools.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon_NameReader.h"
#include "Pokemon_TrainPokemonOCR.h"

namespace PokemonAutomation{
namespace Pokemon{


TrainPokemonOCR_Descriptor::TrainPokemonOCR_Descriptor()
    : ComputerProgramDescriptor(
        "PokemonSwSh:TrainPokemonNameOCR",
        STRING_POKEMON, "Train " + STRING_POKEMON + " Name OCR",
        "",
        "Train " + STRING_POKEMON + " Name OCR"
    )
{}



TrainPokemonOCR::TrainPokemonOCR()
    : DIRECTORY(
        false,
        "<b>Training Data Directory:</b> (Relative to \"TrainingData/\")",
        "PokemonNameOCR/",
        "PokemonNameOCR/"
    )
    , THREADS(
        "<b>Worker Threads:</b>",
        LockWhileRunning::LOCKED,
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
        MODE == TrainOCRMode::Incremental,
        THREADS,
        OCR::BLACK_OR_WHITE_TEXT_FILTERS(),
        PokemonNameReader::MAX_LOG10P + 1.0,
        PokemonNameReader::MAX_LOG10P_SPREAD
    );
}


}
}

