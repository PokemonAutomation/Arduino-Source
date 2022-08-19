/*  IV Checker Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_IVCheckerOption_H
#define PokemonAutomation_Pokemon_IVCheckerOption_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Pokemon/Inference/Pokemon_IVCheckerReader.h"

namespace PokemonAutomation{
namespace Pokemon{




class IVCheckerFilterCell : public EnumDropdownCell<IVCheckerFilter>{
public:
    IVCheckerFilterCell(IVCheckerFilter default_value);
};


class IVCheckerFilterOption : public EnumDropdownOption<IVCheckerFilter>{
public:
    IVCheckerFilterOption(std::string label, IVCheckerFilter default_value = IVCheckerFilter::Anything);

    bool matches(std::atomic<uint64_t>& errors, IVCheckerValue result) const;
};




}
}
#endif
