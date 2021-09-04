/*  Floating-Point Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_FloatingPoint_H
#define PokemonAutomation_FloatingPoint_H

#include "Common/Qt/Options/FloatingPointOption.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class FloatingPoint : public ConfigOption, public FloatingPointOption{
public:
    FloatingPoint(
        QString label,
        double default_value,
        double min_value = -std::numeric_limits<double>::max(),
        double max_value = std::numeric_limits<double>::max()
    )
        : FloatingPointOption(std::move(label), min_value, max_value, default_value)
    {}
    virtual void load_json(const QJsonValue& json) override{
        return this->load_current(json);
    }
    virtual QJsonValue to_json() const override{
        return this->write_current();
    }

    virtual bool is_valid() const override{
        return FloatingPointOption::is_valid();
    }
    virtual void restore_defaults() override{
        FloatingPointOption::restore_defaults();
    }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;
};


class FloatingPointUI : public ConfigOptionUI, public FloatingPointOptionUI{
public:
    FloatingPointUI(QWidget& parent, FloatingPoint& value)
        : FloatingPointOptionUI(parent, value)
    {}
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override{
        FloatingPointOptionUI::restore_defaults();
    }
};


inline ConfigOptionUI* FloatingPoint::make_ui(QWidget& parent){
    return new FloatingPointUI(parent, *this);
}



}
#endif

