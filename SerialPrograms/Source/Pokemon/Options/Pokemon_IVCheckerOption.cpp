/*  IV Checker Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Pokemon/Pokemon_IVChecker.h"
#include "Pokemon_IVCheckerOption.h"

namespace PokemonAutomation{
namespace Pokemon{



IVCheckerFilterOption::IVCheckerFilterOption(std::string label, size_t default_index)
    : EnumDropdownOption(
        std::move(label),
        IVCheckerFilter_NAMES,
        default_index
    )
{}
IVCheckerFilterOption::operator IVCheckerFilter() const{
    return (IVCheckerFilter)(size_t)*this;
}

bool IVCheckerFilterOption::matches(std::atomic<uint64_t>& errors, IVCheckerValue result) const{
    if (result == IVCheckerValue::UnableToDetect){
        errors++;
    }
    return IVChecker_filter_match(*this, result);
}



}
}
