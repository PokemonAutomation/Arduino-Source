/*  Boolean Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_BooleanCheckBoxOption_H
#define PokemonAutomation_BooleanCheckBoxOption_H

#include <atomic>
#include "ConfigOption.h"

namespace PokemonAutomation{


class BooleanCheckBoxOption : public ConfigOption{
public:
    BooleanCheckBoxOption(
        std::string label,
        bool default_value
    );

    const std::string& label() const{ return m_label; }
    operator bool() const{ return m_current.load(std::memory_order_relaxed); }
    void operator=(bool x);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    const std::string m_label;
    const bool m_default;
    std::atomic<bool> m_current;
};





}
#endif

