/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_StringOption_H
#define PokemonAutomation_StringOption_H

#include "Common/Qt/Options/StringOptionBase.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class StringOption : public ConfigOption, public StringOptionBase{
public:
    StringOption(
        QString& backing,
        QString label,
        QString default_value
    )
        : StringOptionBase(backing, std::move(label), default_value)
    {}
    StringOption(
        QString label,
        QString default_value
    )
        : StringOptionBase(std::move(label), default_value)
    {}

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
};


class StringOptionUI : public ConfigOptionUI, public StringOptionBaseUI{
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

