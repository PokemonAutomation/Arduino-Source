/*  Pokemon Nature Checker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_NatureChecker_H
#define PokemonAutomation_Pokemon_NatureChecker_H

#include <string>
#include "Common/Cpp/EnumStringMap.h"
#include "Common/Cpp/Options/EnumDropdownDatabase.h"

namespace PokemonAutomation{
namespace Pokemon{

enum class NatureCheckerValue{
    UnableToDetect,
//    Any,

    Neutral,

    Bashful,
    Docile,
    Hardy,
    Quirky,
    Serious,

    Bold,
    Modest,
    Calm,
    Timid,

    Lonely,
    Mild,
    Gentle,
    Hasty,

    Adamant,
    Impish,
    Careful,
    Jolly,

    Naughty,
    Lax,
    Rash,
    Naive,

    Brave,
    Relaxed,
    Quiet,
    Sassy,
};
const EnumStringMap<NatureCheckerValue>& NATURE_CHECKER_VALUE_STRINGS();
const EnumDropdownDatabase<NatureCheckerValue>& NatureCheckerValue_Database();
NatureCheckerValue NatureCheckerValue_helphinder_to_enum(const std::pair<int,int>& token);


enum class NatureCheckerFilter{
    Any,

    Bashful,
    Docile,
    Hardy,
    Quirky,
    Serious,

    Bold,
    Modest,
    Calm,
    Timid,

    Lonely,
    Mild,
    Gentle,
    Hasty,

    Adamant,
    Impish,
    Careful,
    Jolly,

    Naughty,
    Lax,
    Rash,
    Naive,

    Brave,
    Relaxed,
    Quiet,
    Sassy,
};

const EnumDropdownDatabase<NatureCheckerFilter>& NatureCheckerFilter_Database();
bool NatureChecker_filter_match(NatureCheckerFilter filter, NatureCheckerValue value);

}
}
#endif
