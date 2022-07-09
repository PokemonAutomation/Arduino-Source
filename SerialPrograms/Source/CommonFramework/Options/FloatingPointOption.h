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
#include "Common/Qt/Options/FloatingPoint/FloatingPointBaseOption.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class FloatingPointOption : public ConfigOption, private FloatingPointBaseOption{
public:
    FloatingPointOption(
        QString label,
        double default_value,
        double min_value = -std::numeric_limits<double>::max(),
        double max_value = std::numeric_limits<double>::max()
    );

    using FloatingPointBaseOption::label;
    using FloatingPointBaseOption::operator double;
    using FloatingPointBaseOption::get;
    using FloatingPointBaseOption::set;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual QString check_validity() const override;
    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class FloatingPointWidget;
};




}
#endif

