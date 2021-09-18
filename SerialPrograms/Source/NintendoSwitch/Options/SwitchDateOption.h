/*  Switch Date
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SwitchDate_H
#define PokemonAutomation_SwitchDate_H

#include "Common/Qt/Options/SwitchDateOptionBase.h"
#include "CommonFramework/Options/ConfigOption.h"

namespace PokemonAutomation{


class SwitchDateOption : public ConfigOption, public SwitchDateOptionBase{
public:
    SwitchDateOption(
        QString label,
        QDate default_value
    )
        : SwitchDateOptionBase(std::move(label), default_value)
    {}
    virtual void load_json(const QJsonValue& json) override{
        return this->load_current(json);
    }
    virtual QJsonValue to_json() const override{
        return this->write_current();
    }

    virtual bool is_valid() const override{
        return SwitchDateOptionBase::is_valid();
    }
    virtual void restore_defaults() override{
        SwitchDateOptionBase::restore_defaults();
    }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;
};


class SwitchDateOptionUI : public ConfigOptionUI, public SwitchDateOptionBaseUI{
public:
    SwitchDateOptionUI(QWidget& parent, SwitchDateOption& value)
        : SwitchDateOptionBaseUI(parent, value)
    {}
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override{
        SwitchDateOptionBaseUI::restore_defaults();
    }
};


inline ConfigOptionUI* SwitchDateOption::make_ui(QWidget& parent){
    return new SwitchDateOptionUI(parent, *this);
}



}
#endif

