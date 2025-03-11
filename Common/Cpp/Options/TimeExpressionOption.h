/*  Time Expression Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_TimeExpressionOption_H
#define PokemonAutomation_Options_TimeExpressionOption_H

//#include "Common/Cpp/Time.h"
#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Options/ConfigOption.h"

namespace PokemonAutomation{


std::string ticks_to_time(double ticks_per_second, int64_t ticks);



template <typename Type>
class TimeExpressionCell : public ConfigOption{
public:
    ~TimeExpressionCell();
    TimeExpressionCell(const TimeExpressionCell& x) = delete;
    TimeExpressionCell(
        LockMode lock_while_running,
        double ticks_per_second,
        Type min_value, Type max_value,
        std::string default_value
    );

public:
    TimeExpressionCell(
        LockMode lock_while_running,
        double ticks_per_second,
        std::string default_value
    );
    TimeExpressionCell(
        LockMode lock_while_running,
        double ticks_per_second,
        Type min_value,
        std::string default_value
    );

    double ticks_per_second() const;
    Type min_value() const;
    Type max_value() const;
    const std::string& default_value() const;
    std::string current_text() const;

    operator Type() const;
//    operator Milliseconds() const;
    Type get() const;
    std::string set(std::string text);

    std::string time_string() const;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

protected:
    struct Data;
    Pimpl<Data> m_data;
};



template <typename Type>
class TimeExpressionOption : public TimeExpressionCell<Type>{
public:
    TimeExpressionOption(const TimeExpressionOption& x) = delete;
    TimeExpressionOption(
        std::string label,
        LockMode lock_while_running,
        double ticks_per_second,
        Type min_value, Type max_value,
        std::string default_value
    );

public:
    TimeExpressionOption(
        std::string label,
        LockMode lock_while_running,
        double ticks_per_second,
        std::string default_value
    );
    TimeExpressionOption(
        std::string label,
        LockMode lock_while_running,
        double ticks_per_second,
        Type min_value,
        std::string default_value
    );

    const std::string& label() const{ return m_label; }

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    const std::string m_label;
};




}
#endif


