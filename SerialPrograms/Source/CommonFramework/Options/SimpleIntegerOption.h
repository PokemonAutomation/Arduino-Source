/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SimpleInteger_H
#define PokemonAutomation_SimpleInteger_H

#include "Common/Qt/Options/SimpleIntegerOptionBase.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


template <typename Type>
class SimpleIntegerOption : public ConfigOption, public SimpleIntegerOptionBase<Type>{
public:
    SimpleIntegerOption(
        Type& backing,
        QString label,
        Type default_value,
        Type min_value = std::numeric_limits<Type>::min(),
        Type max_value = std::numeric_limits<Type>::max()
    )
        : SimpleIntegerOptionBase<Type>(backing, std::move(label), min_value, max_value, default_value)
    {}
    SimpleIntegerOption(
        QString label,
        Type default_value,
        Type min_value = std::numeric_limits<Type>::min(),
        Type max_value = std::numeric_limits<Type>::max()
    )
        : SimpleIntegerOptionBase<Type>(std::move(label), min_value, max_value, default_value)
    {}

    virtual void load_json(const QJsonValue& json) override{
        return this->load_current(json);
    }
    virtual QJsonValue to_json() const override{
        return this->write_current();
    }

    virtual bool is_valid() const override{
        return SimpleIntegerOptionBase<Type>::is_valid();
    }
    virtual void restore_defaults() override{
        SimpleIntegerOptionBase<Type>::restore_defaults();
    }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;
};


template <typename Type>
class SimpleIntegerOptionUI : public ConfigOptionUI, public SimpleIntegerOptionBaseUI<Type>{
public:
    SimpleIntegerOptionUI(QWidget& parent, SimpleIntegerOption<Type>& value)
        : SimpleIntegerOptionBaseUI<Type>(parent, value)
    {}
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override{
        SimpleIntegerOptionBaseUI<Type>::restore_defaults();
    }
};


template <typename Type>
inline ConfigOptionUI* SimpleIntegerOption<Type>::make_ui(QWidget& parent){
    return new SimpleIntegerOptionUI<Type>(parent, *this);
}


}
#endif

