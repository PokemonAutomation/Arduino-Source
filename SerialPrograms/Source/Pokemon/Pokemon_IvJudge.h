/*  Pokemon Iv Judge
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_IvJudge_H
#define PokemonAutomation_Pokemon_IvJudge_H

#include <string>
#include "Common/Cpp/Options/EnumDropdownDatabase.h"

namespace PokemonAutomation{
namespace Pokemon{


enum class IvJudgeValue{
    UnableToDetect,
    NoGood,
    Decent,
    PrettyGood,
    VeryGood,
    Fantastic,
    Best,
    HyperTrained,
};
const EnumDropdownDatabase<IvJudgeValue>& IvJudgeValue_Database();
IvJudgeValue IvJudgeValue_string_to_enum(const std::string& token);


enum class IvJudgeFilter{
    Anything,
    NoGood,
    Decent,
    PrettyGood,
    VeryGood,
    Fantastic,
    Best,
};
const EnumDropdownDatabase<IvJudgeFilter>& IvJudgeFilter_Database();
bool IvJudge_filter_match(IvJudgeFilter filter, IvJudgeValue value);



}
}
#endif
