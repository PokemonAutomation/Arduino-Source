/*  Floating-Point Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_FloatingPointOption_H
#define PokemonAutomation_Options_FloatingPointOption_H

#include <limits>
#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Options/ConfigOption.h"

namespace PokemonAutomation{


class FloatingPointCell : public ConfigOption{
public:
    ~FloatingPointCell();
    FloatingPointCell(const FloatingPointCell& x);
    FloatingPointCell(
        LockMode lock_while_running,
        double min_value, double max_value,
        double default_value, double current_value
    );

public:
    FloatingPointCell(
        LockMode lock_while_running,
        double default_value,
        double min_value = -std::numeric_limits<double>::max(),
        double max_value = std::numeric_limits<double>::max()
    );

    double min_value() const;
    double max_value() const;
    double default_value() const;

    operator double() const;
    std::string set(double x);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    std::string check_validity(double x) const;
    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    struct Data;
    Pimpl<Data> m_data;
};


class FloatingPointOption : public FloatingPointCell{
public:
    FloatingPointOption(const FloatingPointOption& x) = delete;

    FloatingPointOption(
        std::string label,
        LockMode lock_while_running,
        double default_value,
        double min_value = -std::numeric_limits<double>::max(),
        double max_value = std::numeric_limits<double>::max()
    );

    const std::string& label() const{ return m_label; }

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    const std::string m_label;
};



}
#endif

