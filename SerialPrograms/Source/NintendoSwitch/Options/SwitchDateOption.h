/*  Switch Date
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_SwitchDate_H
#define PokemonAutomation_SwitchDate_H

#include "Common/Qt/Options/SwitchDateOptionBase.h"
#include "CommonFramework/Options/ConfigOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SwitchDateOption : public ConfigOption, private SwitchDateOptionBase{
public:
    SwitchDateOption(
        QString label,
        QDate default_value
    )
        : SwitchDateOptionBase(std::move(label), default_value)
    {}

    using SwitchDateOptionBase::label;
    using SwitchDateOptionBase::operator QDate;
    using SwitchDateOptionBase::get;
    using SwitchDateOptionBase::set;

    virtual void load_json(const QJsonValue& json) override{
        return this->load_current(json);
    }
    virtual QJsonValue to_json() const override{
        return this->write_current();
    }

    virtual QString check_validity() const override{
        return SwitchDateOptionBase::check_validity();
    }
    virtual void restore_defaults() override{
        SwitchDateOptionBase::restore_defaults();
    }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    friend class SwitchDateOptionUI;
};


class SwitchDateOptionUI : private SwitchDateOptionBaseUI, public ConfigOptionUI{
public:
    SwitchDateOptionUI(QWidget& parent, SwitchDateOption& value)
        : SwitchDateOptionBaseUI(parent, value)
        , ConfigOptionUI(value, *this)
    {}
    virtual void restore_defaults() override{
        SwitchDateOptionBaseUI::restore_defaults();
    }
};


inline ConfigOptionUI* SwitchDateOption::make_ui(QWidget& parent){
    return new SwitchDateOptionUI(parent, *this);
}



}
}
#endif

