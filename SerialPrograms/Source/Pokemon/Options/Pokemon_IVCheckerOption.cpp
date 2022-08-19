/*  IV Checker Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Pokemon/Pokemon_IVChecker.h"
#include "Pokemon_IVCheckerOption.h"

namespace PokemonAutomation{
namespace Pokemon{


const EnumDatabase<IVCheckerFilter>& IVCheckerFilter_Database(){
    static EnumDatabase<IVCheckerFilter> database({
        {IVCheckerFilter::Anything,     "anything",     "Anything (0-31)"},
        {IVCheckerFilter::NoGood,       "no-good",      "No Good (0)"},
        {IVCheckerFilter::Decent,       "decent",       "Decent (1-15)"},
        {IVCheckerFilter::PrettyGood,   "pretty-good",  "Pretty Good (16-25)"},
        {IVCheckerFilter::VeryGood,     "very-good",    "Very Good (26-29)"},
        {IVCheckerFilter::Fantastic,    "fantastic",    "Fantastic (30)"},
        {IVCheckerFilter::Best,         "best",         "Best (31)"},
    });
    return database;
}




IVCheckerFilterCell::IVCheckerFilterCell(IVCheckerFilter default_value)
    : DropdownCell(
        IVCheckerFilter_NAMES,
        (size_t)default_value
    )
{}




IVCheckerFilterOption::IVCheckerFilterOption(std::string label, IVCheckerFilter default_value)
    : EnumDropdownOption<IVCheckerFilter>(
        std::move(label),
        IVCheckerFilter_Database(),
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
