/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_StringOption_H
#define PokemonAutomation_Options_StringOption_H

#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Options/ConfigOption.h"

namespace PokemonAutomation{


class StringCell : public ConfigOption{
public:
    StringCell(
        bool is_password,
        std::string default_value,
        std::string placeholder_text
    );
//    virtual std::unique_ptr<ConfigOption> clone() const override;

    bool is_password() const{ return m_is_password; }
    const std::string& placeholder_text() const{ return m_placeholder_text; }

    operator std::string() const;
    void set(std::string x);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    const bool m_is_password;
    const std::string m_default;
    const std::string m_placeholder_text;

    mutable SpinLock m_lock;
    std::string m_current;
};


class StringOption : public StringCell{
public:
    StringOption(
        bool is_password,
        std::string label,
        std::string default_value,
        std::string placeholder_text
    );

    const std::string& label() const{ return m_label; }

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    const std::string m_label;
};




}
#endif

