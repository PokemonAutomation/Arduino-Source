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


class IVCheckerFilterCell : public DropdownCell{
public:
    IVCheckerFilterCell(IVCheckerFilter default_value);

    operator IVCheckerFilter() const{
        return (IVCheckerFilter)(size_t)*this;
    }
    void set(IVCheckerFilter filter){
        DropdownCell::set_index((size_t)filter);
    }
};


class IVCheckerFilterOption : public DropdownOption{
public:
    IVCheckerFilterOption(std::string label, size_t default_index = 0);

    operator IVCheckerFilter() const;

    bool matches(std::atomic<uint64_t>& errors, IVCheckerValue result) const;
};



}
}
#endif
