/*  Text Edit Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_TextEditOption_H
#define PokemonAutomation_Options_TextEditOption_H

#include "Common/Cpp/SpinLock.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class TextEditOption : public ConfigOption{
public:
    TextEditOption(
        std::string label,
        std::string default_value,
        std::string placeholder_text
    );
//    virtual std::unique_ptr<ConfigOption> clone() const override;

    const std::string& label() const{ return m_label; }
    const std::string& placeholder_text() const{ return m_placeholder_text; }

    operator const std::string&() const;
    void set(std::string x);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    const std::string m_label;
    const std::string m_default;
    const std::string m_placeholder_text;

    mutable SpinLock m_lock;
    std::string m_current;
};




}
#endif
