/*  Boolean Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_BooleanCheckBox_H
#define PokemonAutomation_BooleanCheckBox_H

#include "Common/Qt/Options/BooleanCheckBoxOptionBase.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class BooleanCheckBoxOption : public ConfigOption, public BooleanCheckBoxOptionBase{
public:
    BooleanCheckBoxOption(
        bool& backing,
        QString label,
        bool default_value
    )
        : BooleanCheckBoxOptionBase(backing, std::move(label), default_value)
    {}
    BooleanCheckBoxOption(
        QString label,
        bool default_value
    )
        : BooleanCheckBoxOptionBase(std::move(label), default_value)
    {}

    virtual void load_json(const QJsonValue& json) override{
        load_current(json);
    }
    virtual QJsonValue to_json() const override{
        return write_current();
    }

    virtual void restore_defaults() override{
        BooleanCheckBoxOptionBase::restore_defaults();
    }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;
};


class BooleanCheckBoxOptionUI : public ConfigOptionUI, public BooleanCheckBoxOptionBaseUI{
public:
    BooleanCheckBoxOptionUI(QWidget& parent, BooleanCheckBoxOption& value)
        : BooleanCheckBoxOptionBaseUI(parent, value)
    {}
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override{
        BooleanCheckBoxOptionBaseUI::restore_defaults();
    }
};


inline ConfigOptionUI* BooleanCheckBoxOption::make_ui(QWidget& parent){
    return new BooleanCheckBoxOptionUI(parent, *this);
}


}
#endif

