/*  Pokemon IV Checker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_IVChecker_H
#define PokemonAutomation_Pokemon_IVChecker_H

#include <string>
#include <vector>
#include "Common/Cpp/EnumDatabase.h"

namespace PokemonAutomation{
namespace Pokemon{


enum class IVCheckerValue{
    UnableToDetect,
    NoGood,
    Decent,
    PrettyGood,
    VeryGood,
    Fantastic,
    Best,
    HyperTrained,
};
const EnumDatabase<IVCheckerValue>& IVCheckerValue_Database();
IVCheckerValue IVCheckerValue_string_to_enum(const std::string& token);


enum class IVCheckerFilter{
    Anything,
    NoGood,
    Decent,
    PrettyGood,
    VeryGood,
    Fantastic,
    Best,
};
const EnumDatabase<IVCheckerFilter>& IVCheckerFilter_Database();
bool IVChecker_filter_match(IVCheckerFilter filter, IVCheckerValue value);



}
}
#endif
