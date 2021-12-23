/*  IV Checker Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Pokemon_IVCheckerReader.h"

namespace PokemonAutomation{
namespace Pokemon{



const IVCheckerReader& IVCheckerReader::instance(){
    static IVCheckerReader reader;
    return reader;
}

IVCheckerReader::IVCheckerReader()
    : SmallDictionaryMatcher("Pokemon/IVCheckerOCR.json")
{}





}
}
