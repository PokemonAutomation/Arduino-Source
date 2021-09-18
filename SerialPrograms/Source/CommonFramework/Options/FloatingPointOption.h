/*  Floating-Point Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_FloatingPoint_H
#define PokemonAutomation_FloatingPoint_H

#include "Common/Qt/Options/FloatingPointOptionBase.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class FloatingPointOption : public ConfigOption, public FloatingPointOptionBase{
public:
    FloatingPointOption(
        QString label,
        double default_value,
        double min_value = -std::numeric_limits<double>::max(),
        double max_value = std::numeric_limits<double>::max()
    )
        : FloatingPointOptionBase(std::move(label), min_value, max_value, default_value)
    {}
    virtual void load_json(const QJsonValue& json) override{
        return this->load_current(json);
    }
    virtual QJsonValue to_json() const override{
        return this->write_current();
    }

    virtual bool is_valid() const override{
        return FloatingPointOptionBase::is_valid();
    }
    virtual void restore_defaults() override{
        FloatingPointOptionBase::restore_defaults();
    }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;
};


class FloatingPointOptionUI : public ConfigOptionUI, public FloatingPointOptionBaseUI{
public:
    FloatingPointOptionUI(QWidget& parent, FloatingPointOption& value)
        : FloatingPointOptionBaseUI(parent, value)
    {}
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override{
        FloatingPointOptionBaseUI::restore_defaults();
    }
};


inline ConfigOptionUI* FloatingPointOption::make_ui(QWidget& parent){
    return new FloatingPointOptionUI(parent, *this);
}



}
#endif

