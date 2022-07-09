/*  Processor Level Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ProcessorLevelPption_H
#define PokemonAutomation_ProcessorLevelPption_H

#include "Common/Cpp/CpuId/CpuId.h"
#include "CommonFramework/Options/EnumDropdownOption.h"

namespace PokemonAutomation{


class ProcessorLevelOption : public EnumDropdownOption{
public:
    ProcessorLevelOption();

    virtual void load_json(const JsonValue2& json) override;
    virtual JsonValue2 to_json() const override;

    void set_global();
    void set_global(int index);

    virtual ConfigWidget* make_ui(QWidget& parent) override;
};



}
#endif
