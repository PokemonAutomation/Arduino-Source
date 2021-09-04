/*  Time Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_TimeExpression_H
#define PokemonAutomation_TimeExpression_H

#include <limits>
#include "Common/Qt/Options/TimeExpressionOption.h"
#include "CommonFramework/Options/ConfigOption.h"

namespace PokemonAutomation{


template <typename Type>
class TimeExpression : public ConfigOption, public TimeExpressionOption<Type>{
public:
    TimeExpression(
        Type& backing,
        QString label,
        QString default_value,
        Type min_value = std::numeric_limits<Type>::min(),
        Type max_value = std::numeric_limits<Type>::max()
    )
        : TimeExpressionOption<Type>(backing, std::move(label), min_value, max_value, default_value)
    {}
    TimeExpression(
        QString label,
        QString default_value,
        Type min_value = std::numeric_limits<Type>::min(),
        Type max_value = std::numeric_limits<Type>::max()
    )
        : TimeExpressionOption<Type>(std::move(label), min_value, max_value, default_value)
    {}

    virtual void load_json(const QJsonValue& json) override{
        this->load_current(json);
    }
    virtual QJsonValue to_json() const override{
        return this->write_current();
    }

    virtual bool is_valid() const override{
        return TimeExpressionOption<Type>::is_valid();
    }
    virtual void restore_defaults() override{
        TimeExpressionOption<Type>::restore_defaults();
    }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;
};


template <typename Type>
class TimeExpressionUI : public ConfigOptionUI, public TimeExpressionOptionUI<Type>{
public:
    TimeExpressionUI(QWidget& parent, TimeExpression<Type>& value)
        : TimeExpressionOptionUI<Type>(parent, value)
    {}
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override{
        return TimeExpressionOptionUI<Type>::restore_defaults();
    }
};


template <typename Type>
ConfigOptionUI* TimeExpression<Type>::make_ui(QWidget& parent){
    return new TimeExpressionUI<Type>(parent, *this);
}



}
#endif


