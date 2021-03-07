/*  Switch Date
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SwitchDate_H
#define PokemonAutomation_SwitchDate_H

#include "Common/Qt/Options/SwitchDateOption.h"
#include "CommonFramework/Options/ConfigOption.h"

namespace PokemonAutomation{


class SwitchDate : public ConfigOption, public SwitchDateOption{
public:
    SwitchDate(
        QString label,
        QDate default_value
    )
        : ConfigOption(label)
        , SwitchDateOption(std::move(label), default_value)
    {}
    virtual void load_json(const QJsonValue& json) override{
        return this->load_current(json);
    }
    virtual QJsonValue to_json() const override{
        return this->write_current();
    }

    virtual bool is_valid() const override{
        return SwitchDateOption::is_valid();
    }
    virtual void restore_defaults() override{
        SwitchDateOption::restore_defaults();
    }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;
};


class SwitchDateUI : public ConfigOptionUI, public SwitchDateOptionUI{
public:
    SwitchDateUI(QWidget& parent, SwitchDate& value)
        : SwitchDateOptionUI(parent, value)
    {}
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override{
        SwitchDateOptionUI::restore_defaults();
    }
};


inline ConfigOptionUI* SwitchDate::make_ui(QWidget& parent){
    return new SwitchDateUI(parent, *this);
}



}
#endif

