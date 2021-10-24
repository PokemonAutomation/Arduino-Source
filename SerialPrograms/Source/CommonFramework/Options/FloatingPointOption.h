/*  Floating-Point Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_FloatingPoint_H
#define PokemonAutomation_FloatingPoint_H

#include "Common/Qt/Options/FloatingPointOptionBase.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class FloatingPointOption : public ConfigOption, private FloatingPointOptionBase{
public:
    FloatingPointOption(
        QString label,
        double default_value,
        double min_value = -std::numeric_limits<double>::max(),
        double max_value = std::numeric_limits<double>::max()
    )
        : FloatingPointOptionBase(std::move(label), min_value, max_value, default_value)
    {}

    using FloatingPointOptionBase::label;
    using FloatingPointOptionBase::operator double;
    using FloatingPointOptionBase::get;
    using FloatingPointOptionBase::set;

    virtual void load_json(const QJsonValue& json) override{
        return this->load_current(json);
    }
    virtual QJsonValue to_json() const override{
        return this->write_current();
    }

    virtual QString check_validity() const override{
        return FloatingPointOptionBase::check_validity();
    }
    virtual void restore_defaults() override{
        FloatingPointOptionBase::restore_defaults();
    }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    friend class FloatingPointOptionUI;
};


class FloatingPointOptionUI : public ConfigOptionUI, private FloatingPointOptionBaseUI{
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

