/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_StringOption_H
#define PokemonAutomation_StringOption_H

#include "Common/Qt/Options/StringOptionBase.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class StringOption : public ConfigOption, private StringOptionBase{
public:
    StringOption(
        bool is_password,
        QString label,
        QString default_value,
        QString placeholder_text
    )
        : StringOptionBase(
            is_password,
            std::move(label),
            default_value,
            std::move(placeholder_text)
        )
    {}

    using StringOptionBase::label;
    using StringOptionBase::operator QString;
    using StringOptionBase::get;
    using StringOptionBase::set;

    virtual void load_json(const QJsonValue& json) override{
        load_current(json);
    }
    virtual QJsonValue to_json() const override{
        return write_current();
    }

    virtual void restore_defaults() override{
        StringOptionBase::restore_defaults();
    }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    friend class StringOptionUI;
};


class StringOptionUI : public ConfigOptionUI, private StringOptionBaseUI{
public:
    StringOptionUI(QWidget& parent, StringOptionBase& value)
        : StringOptionBaseUI(parent, value)
    {}
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override{
        StringOptionBaseUI::restore_defaults();
    }
};


inline ConfigOptionUI* StringOption::make_ui(QWidget& parent){
    return new StringOptionUI(parent, *this);
}


}
#endif

