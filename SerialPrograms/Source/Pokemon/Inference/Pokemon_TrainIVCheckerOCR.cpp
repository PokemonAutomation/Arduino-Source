/*  Train IV Checker OCR Data
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <thread>
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonTools/OCR/OCR_TrainingTools.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon_IvJudgeReader.h"
#include "Pokemon_TrainIVCheckerOCR.h"

namespace PokemonAutomation{
namespace Pokemon{


TrainIVCheckerOCR_Descriptor::TrainIVCheckerOCR_Descriptor()
    : ComputerProgramDescriptor(
        "PokemonSwSh:TrainIVCheckerOCR",
        STRING_POKEMON, "Train IV Checker OCR",
        "",
        "Train IV Checker OCR"
    )
{}



TrainIVCheckerOCR::TrainIVCheckerOCR()
    : DIRECTORY(
        false,
        "<b>Training Data Directory:</b> (Relative to \"TrainingData/\")",
        LockMode::LOCK_WHILE_RUNNING,
        "IVCheckerOCR/",
        "IVCheckerOCR/"
    )
    , THREADS(
        "<b>Worker Threads:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        std::thread::hardware_concurrency()
    )
{
    PA_ADD_OPTION(DIRECTORY);
    PA_ADD_OPTION(MODE);
    PA_ADD_OPTION(THREADS);
}



void TrainIVCheckerOCR::program(ProgramEnvironment& env, CancellableScope& scope){
    OCR::TrainingSession session(env.logger(), scope, DIRECTORY);
    session.generate_small_dictionary(
        "Pokemon/IVCheckerOCR.json",
        "IVCheckerOCR.json",
        MODE == TrainOCRMode::INCREMENTAL, THREADS,
        OCR::BLACK_TEXT_FILTERS(),
        IvJudgeReader::MAX_LOG10P,
        IvJudgeReader::MAX_LOG10P_SPREAD
    );
}



}
}
