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
#include <string>

namespace PokemonAutomation{

class JsonValue;


class FloatingPointBaseOption{
public:
    FloatingPointBaseOption(
        std::string label,
        double min_value,
        double max_value,
        double default_value
    );

    const std::string& label() const{ return m_label; }
    double min_value() const{ return m_min_value; }
    double max_value() const{ return m_max_value; }

    operator double() const{ return m_current.load(std::memory_order_relaxed); }
    double get() const{ return m_current.load(std::memory_order_relaxed); }
    std::string set(double x);

    std::string check_validity() const;
    std::string check_validity(double x) const;
    void restore_defaults();

    void load_default(const JsonValue& json);
    void load_current(const JsonValue& json);
    JsonValue write_default() const;
    JsonValue write_current() const;

protected:
    const std::string m_label;
    const double m_min_value;
    const double m_max_value;
    double m_default;

    std::atomic<double> m_current;
};




}
#endif
