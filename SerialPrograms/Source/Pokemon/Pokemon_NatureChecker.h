/*  Pokemon Nature Checker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_NatureChecker_H
#define PokemonAutomation_Pokemon_NatureChecker_H

#include <string>
#include <vector>
#include "Common/Cpp/EnumDatabase.h"

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
const EnumDatabase<NatureCheckerValue>& NatureCheckerValue_Database();
NatureCheckerValue NatureCheckerValue_string_to_enum(const std::string& token);
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

const EnumDatabase<NatureCheckerFilter>& NatureCheckerFilter_Database();
bool NatureChecker_filter_match(NatureCheckerFilter filter, NatureCheckerValue value);

}
}
#endif
