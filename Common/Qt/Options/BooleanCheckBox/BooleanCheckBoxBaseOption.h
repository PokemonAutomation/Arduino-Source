/*  Boolean Check Box Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_BooleanCheckBoxBaseOption_H
#define PokemonAutomation_BooleanCheckBoxBaseOption_H

#include <atomic>
#include <string>

class QCheckBox;

namespace PokemonAutomation{

class JsonValue;


class BooleanCheckBoxBaseOption{
public:
    BooleanCheckBoxBaseOption(
        std::string label,
        bool default_value
    );

    const std::string& label() const{ return m_label; }

    operator bool() const{ return m_current.load(std::memory_order_relaxed); }
    bool get() const{ return m_current.load(std::memory_order_relaxed); }
    void set(bool x){ m_current.store(x, std::memory_order_relaxed); }

    void load_default(const JsonValue& json);
    void load_current(const JsonValue& json);
    JsonValue write_default() const;
    JsonValue write_current() const;

    void restore_defaults();

private:
    const std::string m_label;
    bool m_default;
    std::atomic<bool> m_current;
};




}
#endif
