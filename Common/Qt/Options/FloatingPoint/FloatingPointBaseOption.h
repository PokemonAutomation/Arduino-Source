/*  Floating-Point Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_FloatingPointBaseOption_H
#define PokemonAutomation_FloatingPointBaseOption_H

#include <atomic>
#include <QString>

class QJsonValue;

namespace PokemonAutomation{

class JsonValue2;


class FloatingPointBaseOption{
public:
    FloatingPointBaseOption(
        QString label,
        double min_value,
        double max_value,
        double default_value
    );

    const QString& label() const{ return m_label; }
    double min_value() const{ return m_min_value; }
    double max_value() const{ return m_max_value; }

    operator double() const{ return m_current.load(std::memory_order_relaxed); }
    double get() const{ return m_current.load(std::memory_order_relaxed); }
    QString set(double x);

    QString check_validity() const;
    QString check_validity(double x) const;
    void restore_defaults();

    void load_default(const JsonValue2& json);
    void load_current(const JsonValue2& json);
    JsonValue2 write_default() const;
    JsonValue2 write_current() const;

protected:
    const QString m_label;
    const double m_min_value;
    const double m_max_value;
    double m_default;

    std::atomic<double> m_current;
};




}
#endif
