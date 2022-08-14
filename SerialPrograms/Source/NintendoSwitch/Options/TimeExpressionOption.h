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
#include "Common/Cpp/Options/ConfigOption.h"
#include "Common/Qt/Options/TimeExpression/TimeExpressionBaseOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


template <typename Type>
class TimeExpressionWidget;


template <typename Type>
class TimeExpressionOption : public ConfigOption, private TimeExpressionBaseOption<Type>{
public:
    TimeExpressionOption(
        std::string label,
        std::string default_value,
        Type min_value = std::numeric_limits<Type>::min(),
        Type max_value = std::numeric_limits<Type>::max()
    );

    using TimeExpressionBaseOption<Type>::label;
    using TimeExpressionBaseOption<Type>::operator Type;
    using TimeExpressionBaseOption<Type>::get;
    using TimeExpressionBaseOption<Type>::set;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class TimeExpressionWidget<Type>;
};




}
}
#endif


