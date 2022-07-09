/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_SimpleIntegerOption_H
#define PokemonAutomation_SimpleIntegerOption_H

#include <limits>
#include "Common/Qt/Options/SimpleInteger/SimpleIntegerBaseOption.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


template <typename Type>
class SimpleIntegerWidget;


template <typename Type>
class SimpleIntegerOption : public ConfigOption, private SimpleIntegerBaseOption<Type>{
public:
    SimpleIntegerOption(
        QString label,
        Type default_value,
        Type min_value = std::numeric_limits<Type>::min(),
        Type max_value = std::numeric_limits<Type>::max()
    );

    using SimpleIntegerBaseOption<Type>::label;
    using SimpleIntegerBaseOption<Type>::operator Type;
    using SimpleIntegerBaseOption<Type>::get;
    using SimpleIntegerBaseOption<Type>::set;

    virtual void load_json(const JsonValue2& json) override;
    virtual JsonValue2 to_json() const override;

    virtual QString check_validity() const override;
    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class SimpleIntegerWidget<Type>;
};




}
#endif

