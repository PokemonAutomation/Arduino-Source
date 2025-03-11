/*  Date Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_DateOption_H
#define PokemonAutomation_Options_DateOption_H

#include "Common/Cpp/DateTime.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Options/ConfigOption.h"

namespace PokemonAutomation{



class DateTimeCell : public ConfigOption{
public:
    enum Level{
        DATE,
        DATE_HOUR_MIN,
        DATE_HOUR_MIN_SEC,
    };

public:
    DateTimeCell(
        LockMode lock_while_running,
        Level level,
        const DateTime& min_value, const DateTime& max_value,
        const DateTime& default_value
    );

    Level level() const{ return m_level; }
    const DateTime& min_value() const{ return m_min_value; }
    const DateTime& max_value() const{ return m_max_value; }
    const DateTime& default_value() const{ return m_default; }

    operator DateTime() const;
    DateTime get() const;
    std::string set(const DateTime& x);

    std::string check_validity(const DateTime& x) const;
    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;

public:
    static DateTime from_json(const JsonValue& json);
    static JsonValue to_json(const DateTime& date);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    bool is_valid(const DateTime& date) const;

private:
    const Level m_level;
    const DateTime m_min_value;
    const DateTime m_max_value;
    const DateTime m_default;

    mutable SpinLock m_lock;
    DateTime m_current;
};


class DateTimeOption : public DateTimeCell{
public:
    DateTimeOption(
        std::string label,
        LockMode lock_while_running,
        Level level,
        const DateTime& min_value, const DateTime& max_value,
        const DateTime& default_value
    );

    const std::string& label() const{ return m_label; }

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    const std::string m_label;
};



}
#endif

