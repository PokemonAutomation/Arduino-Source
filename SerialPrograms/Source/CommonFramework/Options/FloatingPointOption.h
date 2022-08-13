/*  Floating-Point Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_FloatingPoint_H
#define PokemonAutomation_FloatingPoint_H

#include <limits>
#include <atomic>
#include "ConfigOption.h"

namespace PokemonAutomation{


class FloatingPointOption : public ConfigOption{
public:
    FloatingPointOption(
        std::string label,
        double default_value,
        double min_value = -std::numeric_limits<double>::max(),
        double max_value = std::numeric_limits<double>::max()
    );

    const std::string& label() const{ return m_label; }
    double min_value() const{ return m_min_value; }
    double max_value() const{ return m_max_value; }

    operator double() const{ return m_current.load(std::memory_order_relaxed); }
    std::string set(double x);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    std::string check_validity(double x) const;
    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    const std::string m_label;
    const double m_min_value;
    const double m_max_value;
    const double m_default;
    std::atomic<double> m_current;
};




}
#endif

