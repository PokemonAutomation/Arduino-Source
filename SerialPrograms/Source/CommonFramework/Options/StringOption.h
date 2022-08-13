/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_StringOption_H
#define PokemonAutomation_StringOption_H

#include "Common/Cpp/SpinLock.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class StringOption : public ConfigOption{
public:
    StringOption(
        bool is_password,
        std::string label,
        std::string default_value,
        std::string placeholder_text
    );

    bool is_password() const{ return m_is_password; }
    const std::string& label() const{ return m_label; }
    const std::string& placeholder_text() const{ return m_placeholder_text; }

    operator std::string() const;
    void set(std::string x);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    const std::string m_label;
    const bool m_is_password;
    const std::string m_default;
    const std::string m_placeholder_text;

    mutable SpinLock m_lock;
    std::string m_current;
};




}
#endif

