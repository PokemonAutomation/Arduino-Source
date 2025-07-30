/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_StringOption_H
#define PokemonAutomation_Options_StringOption_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Options/ConfigOption.h"

namespace PokemonAutomation{


class StringCell : public ConfigOption{
public:
    ~StringCell();
    StringCell(
        bool is_password,
        LockMode lock_while_program_is_running,
        std::string default_value,
        std::string placeholder_text,
        bool signal_all_text_changes = false
    );

    bool is_password() const;
    const std::string& placeholder_text() const;
    const std::string default_value() const;
    bool signal_all_text_changes() const;

    bool is_locked() const;
    void set_locked(bool locked);

    operator std::string() const;
    void set(std::string x);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

protected:
    virtual void sanitize(std::string& str){}

private:
    struct Data;
    Pimpl<Data> m_data;
};


class StringOption : public StringCell{
public:
    StringOption(
        bool is_password,
        std::string label,
        LockMode lock_while_program_is_running,
        std::string default_value,
        std::string placeholder_text,
        bool signal_all_text_changes = false
    );

    const std::string& label() const{ return m_label; }

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    const std::string m_label;
};




}
#endif

