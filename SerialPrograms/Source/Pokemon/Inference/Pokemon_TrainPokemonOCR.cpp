/*  Train Pokemon Name OCR
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <thread>
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonTools/OCR/OCR_TrainingTools.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
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
        LockMode::LOCK_WHILE_RUNNING,
        "PokemonNameOCR/",
        "PokemonNameOCR/"
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


void TrainPokemonOCR::program(ProgramEnvironment& env, CancellableScope& scope){
    if (MODE == TrainOCRMode::GENERATE_BASELINE){
        for (int c = (int)Language::English; c < (int)Language::EndOfList; c++){
            Language language = (Language)c;

            JsonObject json;

            for (const std::string& slug : NATIONAL_DEX_SLUGS()){
                JsonArray array;
                array.push_back(get_pokemon_name(slug).display_name(language));
                json[slug] = std::move(array);
            }

            json.dump("PokemonOCR-" + language_data(language).code + ".json");
        }
        return;
    }

    OCR::TrainingSession session(env.logger(), scope, DIRECTORY);
    session.generate_large_dictionary(
        "Pokemon/PokemonNameOCR/",
        "PokemonOCR-",
        MODE == TrainOCRMode::INCREMENTAL,
        THREADS,
        OCR::BLACK_OR_WHITE_TEXT_FILTERS(),
        PokemonNameReader::MAX_LOG10P + 1.0,
        PokemonNameReader::MAX_LOG10P_SPREAD
    );
}


}
}

