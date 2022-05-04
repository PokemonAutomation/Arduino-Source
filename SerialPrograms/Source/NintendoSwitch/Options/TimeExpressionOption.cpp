/*  Time Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonValue>
#include "Common/Qt/Options/TimeExpression/TimeExpressionBaseWidget.h"
#include "TimeExpressionOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




template <typename Type>
class TimeExpressionWidget : private TimeExpressionBaseWidget<Type>, public ConfigWidget{
public:
    TimeExpressionWidget(QWidget& parent, TimeExpressionOption<Type>& value)
        : TimeExpressionBaseWidget<Type>(parent, value)
        , ConfigWidget(value, *this)
    {}
    virtual void restore_defaults() override{
        return TimeExpressionBaseWidget<Type>::restore_defaults();
    }
    virtual void update_ui() override{
        return TimeExpressionBaseWidget<Type>::update_ui();
    }
};



template <typename Type>
TimeExpressionOption<Type>::TimeExpressionOption(
    QString label,
    QString default_value,
    Type min_value,
    Type max_value
)
    : TimeExpressionBaseOption<Type>(std::move(label), min_value, max_value, default_value)
{}

template <typename Type>
void TimeExpressionOption<Type>::load_json(const QJsonValue& json){
    this->load_current(json);
}
template <typename Type>
QJsonValue TimeExpressionOption<Type>::to_json() const{
    return this->write_current();
}

template <typename Type>
QString TimeExpressionOption<Type>::check_validity() const{
    return TimeExpressionBaseOption<Type>::check_validity();
}
template <typename Type>
void TimeExpressionOption<Type>::restore_defaults(){
    TimeExpressionBaseOption<Type>::restore_defaults();
}


template <typename Type>
ConfigWidget* TimeExpressionOption<Type>::make_ui(QWidget& parent){
    return new TimeExpressionWidget<Type>(parent, *this);
}


template class TimeExpressionOption<uint16_t>;
template class TimeExpressionOption<uint32_t>;
template class TimeExpressionOption<int16_t>;
template class TimeExpressionOption<int32_t>;



}
}
