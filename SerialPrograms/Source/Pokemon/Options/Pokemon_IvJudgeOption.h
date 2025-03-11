/*  IV Judge Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_IvJudgeOption_H
#define PokemonAutomation_Pokemon_IvJudgeOption_H

#include <atomic>
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Pokemon/Pokemon_IvJudge.h"

namespace PokemonAutomation{
namespace Pokemon{




class IVJudgeFilterCell : public EnumDropdownCell<IvJudgeFilter>{
public:
    IVJudgeFilterCell(IvJudgeFilter default_value);
};


class IVJudgeFilterOption : public EnumDropdownOption<IvJudgeFilter>{
public:
    IVJudgeFilterOption(std::string label, IvJudgeFilter default_value = IvJudgeFilter::Anything);

    bool matches(std::atomic<uint64_t>& errors, IvJudgeValue result) const;
};




}
}
#endif
