/*  Time Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_TimeExpression_H
#define PokemonAutomation_TimeExpression_H

#include <limits>
#include "Common/Qt/Options/TimeExpressionOptionBase.h"
#include "CommonFramework/Options/ConfigOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


template <typename Type>
class TimeExpressionOptionUI;


template <typename Type>
class TimeExpressionOption : public ConfigOption, private TimeExpressionOptionBase<Type>{
public:
    TimeExpressionOption(
        QString label,
        QString default_value,
        Type min_value = std::numeric_limits<Type>::min(),
        Type max_value = std::numeric_limits<Type>::max()
    )
        : TimeExpressionOptionBase<Type>(std::move(label), min_value, max_value, default_value)
    {}

    using TimeExpressionOptionBase<Type>::label;
    using TimeExpressionOptionBase<Type>::operator Type;
    using TimeExpressionOptionBase<Type>::get;
    using TimeExpressionOptionBase<Type>::set;

    virtual void load_json(const QJsonValue& json) override{
        this->load_current(json);
    }
    virtual QJsonValue to_json() const override{
        return this->write_current();
    }

    virtual QString check_validity() const override{
        return TimeExpressionOptionBase<Type>::check_validity();
    }
    virtual void restore_defaults() override{
        TimeExpressionOptionBase<Type>::restore_defaults();
    }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    friend class TimeExpressionOptionUI<Type>;
};


template <typename Type>
class TimeExpressionOptionUI : public ConfigOptionUI, private TimeExpressionOptionBaseUI<Type>{
public:
    TimeExpressionOptionUI(QWidget& parent, TimeExpressionOption<Type>& value)
        : TimeExpressionOptionBaseUI<Type>(parent, value)
    {}
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override{
        return TimeExpressionOptionBaseUI<Type>::restore_defaults();
    }
};


template <typename Type>
ConfigOptionUI* TimeExpressionOption<Type>::make_ui(QWidget& parent){
    return new TimeExpressionOptionUI<Type>(parent, *this);
}



}
}
#endif


