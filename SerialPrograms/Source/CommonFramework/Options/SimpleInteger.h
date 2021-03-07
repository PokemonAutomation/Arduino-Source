/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SimpleInteger_H
#define PokemonAutomation_SimpleInteger_H

#include "Common/Qt/Options/SimpleIntegerOption.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


template <typename Type>
class SimpleInteger : public ConfigOption, public SimpleIntegerOption<Type>{
public:
    SimpleInteger(
        Type& backing,
        QString label,
        Type default_value,
        Type min_value = std::numeric_limits<Type>::min(),
        Type max_value = std::numeric_limits<Type>::max()
    )
        : ConfigOption(label)
        , SimpleIntegerOption<Type>(backing, std::move(label), min_value, max_value, default_value)
    {}
    SimpleInteger(
        QString label,
        Type default_value,
        Type min_value = std::numeric_limits<Type>::min(),
        Type max_value = std::numeric_limits<Type>::max()
    )
        : ConfigOption(label)
        , SimpleIntegerOption<Type>(std::move(label), min_value, max_value, default_value)
    {}

    virtual void load_json(const QJsonValue& json) override{
        return this->load_current(json);
    }
    virtual QJsonValue to_json() const override{
        return this->write_current();
    }

    virtual bool is_valid() const override{
        return SimpleIntegerOption<Type>::is_valid();
    }
    virtual void restore_defaults() override{
        SimpleIntegerOption<Type>::restore_defaults();
    }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;
};


template <typename Type>
class SimpleIntegerUI : public ConfigOptionUI, public SimpleIntegerOptionUI<Type>{
public:
    SimpleIntegerUI(QWidget& parent, SimpleInteger<Type>& value)
        : SimpleIntegerOptionUI<Type>(parent, value)
    {}
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override{
        SimpleIntegerOptionUI<Type>::restore_defaults();
    }
};


template <typename Type>
inline ConfigOptionUI* SimpleInteger<Type>::make_ui(QWidget& parent){
    return new SimpleIntegerUI<Type>(parent, *this);
}


}
#endif

