/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_StringBaseOption_H
#define PokemonAutomation_StringBaseOption_H

#include <string>
#include "Common/Cpp/SpinLock.h"

namespace PokemonAutomation{

class JsonValue;


class StringBaseOption{
public:
    StringBaseOption(
        bool is_password,
        std::string label,
        std::string default_value,
        std::string placeholder_text
    );

    const std::string& label() const{ return m_label; }
    const std::string& placeholder_text() const{ return m_placeholder_text; }
    bool is_password() const{ return m_is_password; }

    operator std::string() const;
    std::string get() const;
    void set(std::string x);

    void restore_defaults();

    void load_default(const JsonValue& json);
    void load_current(const JsonValue& json);
    JsonValue write_default() const;
    JsonValue write_current() const;

private:
    const std::string m_label;
    std::string m_default;
    std::string m_placeholder_text;
    bool m_is_password;

    mutable SpinLock m_lock;
    std::string m_current;
};




}
#endif
