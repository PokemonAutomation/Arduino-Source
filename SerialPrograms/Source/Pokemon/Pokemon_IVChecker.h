/*  Pokemon IV Checker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_IVChecker_H
#define PokemonAutomation_Pokemon_IVChecker_H

#include <string>
#include <vector>

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
IVCheckerValue IVCheckerValue_string_to_enum(const std::string& token);
const std::string& IVCheckerValue_enum_to_string(IVCheckerValue result);


enum class IVCheckerFilter{
    Anything,
    NoGood,
    Decent,
    PrettyGood,
    VeryGood,
    Fantastic,
    Best,
};
extern const std::vector<std::string> IVCheckerFilter_NAMES;
IVCheckerFilter IVCheckerFilter_string_to_enum(const std::string& string);
const std::string& IVCheckerFilter_enum_to_string(IVCheckerFilter result);


bool IVChecker_filter_match(IVCheckerFilter filter, IVCheckerValue value);



}
}
#endif
