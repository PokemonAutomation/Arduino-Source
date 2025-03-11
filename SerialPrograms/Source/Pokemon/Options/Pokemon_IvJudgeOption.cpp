/*  IV Judge Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Pokemon/Pokemon_IvJudge.h"
#include "Pokemon_IvJudgeOption.h"

namespace PokemonAutomation{
namespace Pokemon{




IVJudgeFilterCell::IVJudgeFilterCell(IvJudgeFilter default_value)
    : EnumDropdownCell<IvJudgeFilter>(
        IvJudgeFilter_Database(),
        LockMode::LOCK_WHILE_RUNNING,
        default_value
    )
{}




IVJudgeFilterOption::IVJudgeFilterOption(std::string label, IvJudgeFilter default_value)
    : EnumDropdownOption<IvJudgeFilter>(
        std::move(label),
        IvJudgeFilter_Database(),
        LockMode::LOCK_WHILE_RUNNING,
        default_value
    )
{}

bool IVJudgeFilterOption::matches(std::atomic<uint64_t>& errors, IvJudgeValue result) const{
    if (result == IvJudgeValue::UnableToDetect){
        errors++;
    }
    return IvJudge_filter_match(*this, result);
}



}
}
