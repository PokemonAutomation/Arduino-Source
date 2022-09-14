/*  Text Edit Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_TextEditOption_H
#define PokemonAutomation_Options_TextEditOption_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class TextEditOption : public ConfigOption{
public:
    ~TextEditOption();
    TextEditOption(
        std::string label,
        std::string default_value,
        std::string placeholder_text
    );
//    virtual std::unique_ptr<ConfigOption> clone() const override;

    const std::string& label() const;
    const std::string& placeholder_text() const;

    operator std::string() const;
    void set(std::string x);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    struct Data;
    Pimpl<Data> m_data;
};




}
#endif
