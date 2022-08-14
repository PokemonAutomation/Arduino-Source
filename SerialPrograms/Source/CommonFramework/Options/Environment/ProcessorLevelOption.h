/*  Processor Level Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ProcessorLevelPption_H
#define PokemonAutomation_ProcessorLevelPption_H

#include "Common/Cpp/CpuId/CpuId.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{


class ProcessorLevelOption : public EnumDropdownOption{
public:
    ProcessorLevelOption();

    virtual void set(size_t index) override;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    void set_global();
    void set_global(size_t index);
};



}
#endif
