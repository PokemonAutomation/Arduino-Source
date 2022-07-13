/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_SimpleIntegerBaseOption_H
#define PokemonAutomation_SimpleIntegerBaseOption_H

#include <atomic>
#include <string>

namespace PokemonAutomation{

class JsonValue;


template <typename Type>
class SimpleIntegerBaseOption{
public:
    SimpleIntegerBaseOption(
        std::string label,
        Type min_value,
        Type max_value,
        Type default_value
    );

    const std::string& label() const{ return m_label; }

    operator Type() const{ return m_current.load(std::memory_order_relaxed); }
    Type get() const{ return m_current.load(std::memory_order_relaxed); }
    std::string set(Type x);

    std::string check_validity() const;
    std::string check_validity(Type x) const;
    void restore_defaults();

    void load_default(const JsonValue& json);
    void load_current(const JsonValue& json);
    JsonValue write_default() const;
    JsonValue write_current() const;


protected:
    const std::string m_label;
    const Type m_min_value;
    const Type m_max_value;
    Type m_default;
    std::atomic<Type> m_current;
};




}
#endif
