/*  Boolean Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_BooleanCheckBox_H
#define PokemonAutomation_BooleanCheckBox_H

#include "Common/Qt/Options/BooleanCheckBoxOption.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class BooleanCheckBox : public ConfigOption, public BooleanCheckBoxOption{
public:
    BooleanCheckBox(
        bool& backing,
        QString label,
        bool default_value
    )
        : BooleanCheckBoxOption(backing, std::move(label), default_value)
    {}
    BooleanCheckBox(
        QString label,
        bool default_value
    )
        : BooleanCheckBoxOption(std::move(label), default_value)
    {}

    virtual void load_json(const QJsonValue& json) override{
        load_current(json);
    }
    virtual QJsonValue to_json() const override{
        return write_current();
    }

    virtual void restore_defaults() override{
        BooleanCheckBoxOption::restore_defaults();
    }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;
};


class BooleanCheckBoxUI : public ConfigOptionUI, public BooleanCheckBoxOptionUI{
public:
    BooleanCheckBoxUI(QWidget& parent, BooleanCheckBox& value)
        : BooleanCheckBoxOptionUI(parent, value)
    {}
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override{
        BooleanCheckBoxOptionUI::restore_defaults();
    }
};


inline ConfigOptionUI* BooleanCheckBox::make_ui(QWidget& parent){
    return new BooleanCheckBoxUI(parent, *this);
}


}
#endif

