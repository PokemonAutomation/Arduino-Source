/*  Egg Step Count Dropdown
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EggStepCount_H
#define PokemonAutomation_PokemonSwSh_EggStepCount_H

#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{

extern const std::vector<uint16_t> STEP_COUNTS;


class EggStepCountOption : public ConfigOption{
public:
    EggStepCountOption();
    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    operator uint16_t() const{ return STEP_COUNTS[m_option]; }

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    EnumDropdownOption m_option;
};



}
#endif

