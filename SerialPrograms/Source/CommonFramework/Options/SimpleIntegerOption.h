/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_SimpleInteger_H
#define PokemonAutomation_SimpleInteger_H

#include <limits>
#include "Common/Qt/Options/SimpleIntegerOptionBase.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


template <typename Type>
class SimpleIntegerOptionUI;


template <typename Type>
class SimpleIntegerOption : public ConfigOption, private SimpleIntegerOptionBase<Type>{
public:
    SimpleIntegerOption(
        QString label,
        Type default_value,
        Type min_value = std::numeric_limits<Type>::min(),
        Type max_value = std::numeric_limits<Type>::max()
    )
        : SimpleIntegerOptionBase<Type>(std::move(label), min_value, max_value, default_value)
    {}

    using SimpleIntegerOptionBase<Type>::label;
    using SimpleIntegerOptionBase<Type>::operator Type;
    using SimpleIntegerOptionBase<Type>::get;
    using SimpleIntegerOptionBase<Type>::set;

    virtual void load_json(const QJsonValue& json) override{
        return this->load_current(json);
    }
    virtual QJsonValue to_json() const override{
        return this->write_current();
    }

    virtual QString check_validity() const override{
        return SimpleIntegerOptionBase<Type>::check_validity();
    }
    virtual void restore_defaults() override{
        SimpleIntegerOptionBase<Type>::restore_defaults();
    }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    friend class SimpleIntegerOptionUI<Type>;
};


template <typename Type>
class SimpleIntegerOptionUI : private SimpleIntegerOptionBaseUI<Type>, public ConfigOptionUI{
public:
    SimpleIntegerOptionUI(QWidget& parent, SimpleIntegerOption<Type>& value)
        : SimpleIntegerOptionBaseUI<Type>(parent, value)
        , ConfigOptionUI(value, *this)
    {}
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

