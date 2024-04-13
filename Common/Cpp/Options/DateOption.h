/*  Date Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_DateOption_H
#define PokemonAutomation_Options_DateOption_H

//#include <QDate>
#include "Common/Cpp/DateTime.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Options/ConfigOption.h"

namespace PokemonAutomation{


#if 0
class DateOption : public ConfigOption{
public:
    DateOption(
        std::string label,
        QDate min_value, QDate max_value,
        QDate default_value
    );
//    virtual std::unique_ptr<ConfigOption> clone() const override;

    const std::string& label() const{ return m_label; }
    const QDate& min_value() const{ return m_min_value; }
    const QDate& max_value() const{ return m_max_value; }
    const QDate& default_value() const{ return m_default; }

    operator QDate() const;
    QDate get() const;
    std::string set(QDate x);

    static QDate from_json(const JsonValue& json);
    static JsonValue to_json(const QDate& date);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    std::string check_validity(QDate x) const;
    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    const std::string m_label;
    const QDate m_min_value;
    const QDate m_max_value;
    const QDate m_default;

    mutable SpinLock m_lock;
    QDate m_current;
};
#endif


class DateTimeOption : public ConfigOption{
public:
    enum Level{
        DATE,
        DATE_HOUR_MIN,
        DATE_HOUR_MIN_SEC,
    };

public:
    DateTimeOption(
        std::string label,
        LockMode lock_while_running,
        Level level,
        const DateTime& min_value, const DateTime& max_value,
        const DateTime& default_value
    );

    const std::string& label() const{ return m_label; }
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
    const std::string m_label;
    const Level m_level;
    const DateTime m_min_value;
    const DateTime m_max_value;
    const DateTime m_default;

    mutable SpinLock m_lock;
    DateTime m_current;
};





}
#endif

