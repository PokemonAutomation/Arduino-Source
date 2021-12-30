/*  Train IV Checker OCR Data
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
#include "CommonFramework/OCR/OCR_TextMatcher.h"
#include "CommonFramework/OCR/OCR_TrainingTools.h"
#include "Pokemon_TrainIVCheckerOCR.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Pokemon{


TrainIVCheckerOCR_Descriptor::TrainIVCheckerOCR_Descriptor()
    : RunnableComputerProgramDescriptor(
        "PokemonSwSh:TrainIVCheckerOCR",
        STRING_POKEMON, "Train IV Checker OCR",
        "",
        "Train IV Checker OCR"
    )
{}



TrainIVCheckerOCR::TrainIVCheckerOCR(const TrainIVCheckerOCR_Descriptor& descriptor)
    : RunnableComputerProgramInstance(descriptor)
    , DIRECTORY(
        false,
        "<b>Training Data Directory:</b> (Relative to \"TrainingData/\")",
        "IVCheckerOCR/",
        "IVCheckerOCR/"
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



void TrainIVCheckerOCR::program(ProgramEnvironment& env){
    OCR::TrainingSession session(env, DIRECTORY);
    session.generate_small_dictionary(
        env,
        "PokemonSwSh/IVCheckerOCR.json",
        "IVCheckerOCR.json",
        MODE != 0,
        THREADS
    );
}



}
}
