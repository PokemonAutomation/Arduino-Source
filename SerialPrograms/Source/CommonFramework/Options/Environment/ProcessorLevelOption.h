/*  Processor Level Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ProcessorLevelOption_H
#define PokemonAutomation_ProcessorLevelOption_H

#include "Common/Cpp/CpuId/CpuId.h"
#include "Common/Cpp/Options/DropdownOption.h"

namespace PokemonAutomation{


class ProcessorLevelOption : public DropdownOption{
public:
    ProcessorLevelOption();

    virtual bool set_index(size_t index) override;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    void set_global();
    void set_global(size_t index);
};



}
#endif
