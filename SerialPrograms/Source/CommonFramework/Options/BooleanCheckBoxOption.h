/*  Boolean Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_BooleanCheckBox_H
#define PokemonAutomation_BooleanCheckBox_H

#include "Common/Qt/Options/BooleanCheckBoxOptionBase.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class BooleanCheckBoxOption : public ConfigOption, private BooleanCheckBoxOptionBase{
public:
    BooleanCheckBoxOption(
        QString label,
        bool default_value
    )
        : BooleanCheckBoxOptionBase(std::move(label), default_value)
    {}

    using BooleanCheckBoxOptionBase::label;
    using BooleanCheckBoxOptionBase::operator bool;
    using BooleanCheckBoxOptionBase::get;
    using BooleanCheckBoxOptionBase::set;

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

private:
    friend class BooleanCheckBoxOptionUI;
};


class BooleanCheckBoxOptionUI : private BooleanCheckBoxOptionBaseUI, public ConfigOptionUI{
public:
    BooleanCheckBoxOptionUI(QWidget& parent, BooleanCheckBoxOption& value)
        : BooleanCheckBoxOptionBaseUI(parent, value)
        , ConfigOptionUI(value, *this)
    {}
    virtual void restore_defaults() override{
        BooleanCheckBoxOptionBaseUI::restore_defaults();
    }
};


inline ConfigOptionUI* BooleanCheckBoxOption::make_ui(QWidget& parent){
    return new BooleanCheckBoxOptionUI(parent, *this);
}


}
#endif

