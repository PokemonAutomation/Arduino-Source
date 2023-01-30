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
    Neutral,
    Any,
    Adamant,
    Bashful,
    Bold,
    Brave,
    Calm,
    Careful,
    Docile,
    Gentle,
    Hardy,
    Hasty,
    Impish,
    Jolly,
    Lax,
    Lonely,
    Mild,
    Modest,
    Naive,
    Naughty,
    Quiet,
    Quirky,
    Rash,
    Relaxed,
    Sassy,
    Serious,
    Timid
};
const EnumDatabase<NatureCheckerValue>& NatureCheckerValue_Database();
NatureCheckerValue NatureCheckerValue_string_to_enum(const std::string& token);
NatureCheckerValue NatureCheckerValue_helphinder_to_enum(const std::pair<int,int>& token);


enum class NatureCheckerFilter {
    Any,
    Adamant,
    Bashful,
    Bold,
    Brave,
    Calm,
    Careful,
    Docile,
    Gentle,
    Hardy,
    Hasty,
    Impish,
    Jolly,
    Lax,
    Lonely,
    Mild,
    Modest,
    Naive,
    Naughty,
    Quiet,
    Quirky,
    Rash,
    Relaxed,
    Sassy,
    Serious,
    Timid
};

const EnumDatabase<NatureCheckerFilter>& NatureCheckerFilter_Database();
bool NatureChecker_filter_match(NatureCheckerFilter filter, NatureCheckerValue value);

}
}
#endif
