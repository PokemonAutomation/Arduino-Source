/*  Processor Level Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ProcessorLevelOption_H
#define PokemonAutomation_ProcessorLevelOption_H

#include "Common/Cpp/CpuId/CpuId.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{


class ProcessorLevelOption : public IntegerEnumDropdownOption{
public:
    ProcessorLevelOption();

    virtual bool set_value(size_t value) override;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    void set_global();
    void set_global(size_t value);
};



}
#endif
