/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_String_H
#define PokemonAutomation_String_H

#include "Common/Qt/Options/StringOption.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class String : public ConfigOption, public StringOption{
public:
    String(
        QString& backing,
        QString label,
        QString default_value
    )
        : ConfigOption(label)
        , StringOption(backing, std::move(label), default_value)
    {}
    String(
        QString label,
        QString default_value
    )
        : ConfigOption(label)
        , StringOption(std::move(label), default_value)
    {}

    virtual void load_json(const QJsonValue& json) override{
        load_current(json);
    }
    virtual QJsonValue to_json() const override{
        return write_current();
    }

    virtual void restore_defaults() override{
        StringOption::restore_defaults();
    }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;
};


class StringUI : public ConfigOptionUI, public StringOptionUI{
public:
    StringUI(QWidget& parent, StringOption& value)
        : StringOptionUI(parent, value)
    {}
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override{
        StringOptionUI::restore_defaults();
    }
};


inline ConfigOptionUI* String::make_ui(QWidget& parent){
    return new StringUI(parent, *this);
}


}
#endif

