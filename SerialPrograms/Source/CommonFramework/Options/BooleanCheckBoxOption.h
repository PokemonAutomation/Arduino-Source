/*  Boolean Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_BooleanCheckBoxOption_H
#define PokemonAutomation_BooleanCheckBoxOption_H

#include "Common/Qt/Options/BooleanCheckBox/BooleanCheckBoxBaseOption.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class BooleanCheckBoxOption : public ConfigOption, private BooleanCheckBoxBaseOption{
public:
    BooleanCheckBoxOption(
        std::string label,
        bool default_value
    )
        : BooleanCheckBoxBaseOption(std::move(label), default_value)
    {}

    using BooleanCheckBoxBaseOption::label;
    using BooleanCheckBoxBaseOption::operator bool;
    using BooleanCheckBoxBaseOption::get;
    using BooleanCheckBoxBaseOption::set;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class BooleanCheckBoxWidget;
};





}
#endif

