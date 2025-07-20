/*  Time Duration Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_TimeDurationOption_H
#define PokemonAutomation_Options_TimeDurationOption_H

#include "Common/Cpp/Time.h"
#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Options/ConfigOption.h"

namespace PokemonAutomation{


template <typename Type>
class TimeDurationCell : public ConfigOption{
public:
    ~TimeDurationCell();
    TimeDurationCell(const TimeDurationCell& x) = delete;
    TimeDurationCell(
        std::string units, bool show_summary,
        LockMode lock_while_running,
        Type min_value, Type max_value,
        std::string default_value
    );

public:
    TimeDurationCell(
        std::string units,
        LockMode lock_while_running,
        std::string default_value
    );
    TimeDurationCell(
        std::string units,
        LockMode lock_while_running,
        Type min_value,
        std::string default_value
    );
    TimeDurationCell(
        std::string units,
        LockMode lock_while_running,
        Type min_value, Type max_value,
        std::string default_value
    );


public:
    const std::string& units() const;
    bool show_summary() const;
    Type min_value() const;
    Type max_value() const;
    const std::string& default_value() const;
    std::string current_text() const;

    template <class Rep, class Period>
    operator std::chrono::duration<Rep, Period>() const{
        return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(get());
    }

    Type get() const;
    std::string set(std::string text);


public:
    //  Get the description text using the current value.
    std::string time_string() const;

    //  Get the description text if you were to set the box to this value.
    std::string time_string(const std::string& text) const;


public:
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
class TimeDurationOption : public TimeDurationCell<Type>{
public:
    template <class... Args>
    TimeDurationOption(std::string label, Args&&... args)
        : TimeDurationCell<Type>(std::forward<Args>(args)...)
        , m_label(std::move(label))
    {}

    const std::string& label() const{ return m_label; }

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    const std::string m_label;
};





class MillisecondsCell : public TimeDurationCell<std::chrono::milliseconds>{
public:
    template <class... Args>
    MillisecondsCell(Args&&... args)
        : TimeDurationCell<std::chrono::milliseconds>("milliseconds", std::forward<Args>(args)...)
    {}
};
class MillisecondsOption : public TimeDurationOption<std::chrono::milliseconds>{
public:
    template <class... Args>
    MillisecondsOption(std::string label, Args&&... args)
        : TimeDurationOption<std::chrono::milliseconds>(
            std::move(label),
            "milliseconds",
            std::forward<Args>(args)...
        )
    {}
};



class MicrosecondsOption : public TimeDurationOption<std::chrono::microseconds>{
public:
    template <class... Args>
    MicrosecondsOption(std::string label, Args&&... args)
        : TimeDurationOption<std::chrono::microseconds>(
            std::move(label),
            "microseconds",
            std::forward<Args>(args)...
        )
    {}
};




}
#endif
