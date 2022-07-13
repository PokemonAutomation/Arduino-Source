/*  Time Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_TimeExpressionBaseOption_H
#define PokemonAutomation_TimeExpressionBaseOption_H

#include <string>
#include "Common/Cpp/SpinLock.h"

class JsonValue;

namespace PokemonAutomation{
namespace NintendoSwitch{


template <typename Type>
class TimeExpressionBaseOption{
public:
    TimeExpressionBaseOption(
        std::string label,
        Type min_value,
        Type max_value,
        std::string default_value
    );

    const std::string& label() const{ return m_label; }

    operator Type() const;
    Type get() const;
    std::string set(std::string text);

    std::string text() const;
    std::string time_string() const;

    void load_default(const JsonValue& json);
    void load_current(const JsonValue& json);
    JsonValue write_default() const;
    JsonValue write_current() const;

    Type min() const{ return m_min_value; }
    Type max() const{ return m_max_value; }

    std::string check_validity() const;
    void restore_defaults();

private:
    std::string process(const std::string& text, Type& value) const;

private:
    const std::string m_label;
    const Type m_min_value;
    const Type m_max_value;
    std::string m_default;

    mutable SpinLock m_lock;
    std::string m_current;
    Type m_value;
    std::string m_error;
};




}
}
#endif


