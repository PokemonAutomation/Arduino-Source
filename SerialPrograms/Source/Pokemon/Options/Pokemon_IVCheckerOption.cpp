/*  IV Checker Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Pokemon/Pokemon_IVChecker.h"
#include "Pokemon_IVCheckerOption.h"

namespace PokemonAutomation{
namespace Pokemon{




IVCheckerFilterCell::IVCheckerFilterCell(IVCheckerFilter default_value)
    : EnumDropdownCell<IVCheckerFilter>(
        IVCheckerFilter_Database(),
        LockWhileRunning::LOCK_WHILE_RUNNING,
        default_value
    )
{}




IVCheckerFilterOption::IVCheckerFilterOption(std::string label, IVCheckerFilter default_value)
    : EnumDropdownOption<IVCheckerFilter>(
        std::move(label),
        IVCheckerFilter_Database(),
        LockWhileRunning::LOCK_WHILE_RUNNING,
        default_value
    )
{}

bool IVCheckerFilterOption::matches(std::atomic<uint64_t>& errors, IVCheckerValue result) const{
    if (result == IVCheckerValue::UnableToDetect){
        errors++;
    }
    return IVChecker_filter_match(*this, result);
}



}
}
