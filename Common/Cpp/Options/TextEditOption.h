/*  Text Edit Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_TextEditOption_H
#define PokemonAutomation_Options_TextEditOption_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class TextEditOption : public ConfigOptionImpl<TextEditOption>{
public:
    //  Listeners for when the user focuses on this box.
    struct FocusListener{
        virtual void focus_in() = 0;
    };
    void add_focus_listener(FocusListener& listener);
    void remove_focus_listener(FocusListener& listener);
    void report_focus_in();

    struct AppendListener{
        virtual void on_append(std::string text) = 0;
    };
    void add_append_listener(AppendListener& listener);
    void remove_append_listener(AppendListener& listener);


public:
    ~TextEditOption();
    TextEditOption(
        std::string label,
        LockMode lock_while_program_is_running,
        std::string default_value,
        std::string placeholder_text,
        bool signal_all_text_changes = false
    );
//    virtual std::unique_ptr<ConfigOption> clone() const override;

    const std::string& label() const;
    const std::string& placeholder_text() const;
    bool signal_all_text_changes() const;

    operator std::string() const;
    void set(std::string x);

    void append(std::string x);


public:
    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;


private:
    struct Data;
    Pimpl<Data> m_data;
};




}
#endif
