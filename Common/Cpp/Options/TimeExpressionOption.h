/*  Time Expression Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_TimeExpressionOption_H
#define PokemonAutomation_Options_TimeExpressionOption_H

#include <limits>
#include "Common/Cpp/SpinLock.h"
#include "Common/Cpp/Options/ConfigOption.h"

namespace PokemonAutomation{


std::string ticks_to_time(double ticks_per_second, int64_t ticks);



template <typename Type>
class TimeExpressionCell : public ConfigOption{
public:
    TimeExpressionCell(
        double ticks_per_second,
        std::string default_value,
        Type min_value = std::numeric_limits<Type>::min(),
        Type max_value = std::numeric_limits<Type>::max()
    );

    Type min_value() const{ return m_min_value; }
    Type max_value() const{ return m_max_value; }
    const std::string& default_value() const{ return m_default; }

    operator Type() const;
    Type get() const;
    std::string set(std::string text);

    std::string text() const;
    std::string time_string() const;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    std::string process(const std::string& text, Type& value) const;

private:
    const double m_ticks_per_second;
    const Type m_min_value;
    const Type m_max_value;
    const std::string m_default;

    mutable SpinLock m_lock;
    std::string m_current;
    Type m_value;
    std::string m_error;
};



template <typename Type>
class TimeExpressionOption : public TimeExpressionCell<Type>{
public:
    TimeExpressionOption(
        double ticks_per_second,
        std::string label,
        std::string default_value,
        Type min_value = std::numeric_limits<Type>::min(),
        Type max_value = std::numeric_limits<Type>::max()
    );

    const std::string& label() const{ return m_label; }

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    const std::string m_label;
};




}
#endif


