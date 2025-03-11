/*  Pokemon Iv Judge
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_IvJudge_H
#define PokemonAutomation_Pokemon_IvJudge_H

#include "Common/Cpp/EnumStringMap.h"
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
const EnumStringMap<IvJudgeValue>& IV_JUDGE_VALUE_STRINGS();
const EnumDropdownDatabase<IvJudgeValue>& IvJudgeValue_Database();


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
