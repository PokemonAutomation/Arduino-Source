/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_StringOption_H
#define PokemonAutomation_StringOption_H

#include "Common/Qt/Options/String/StringBaseOption.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class StringOption : public ConfigOption, private StringBaseOption{
public:
    StringOption(
        bool is_password,
        QString label,
        QString default_value,
        QString placeholder_text
    );

    using StringBaseOption::label;
    using StringBaseOption::operator QString;
    using StringBaseOption::get;
    using StringBaseOption::set;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class StringWidget;
};




}
#endif

