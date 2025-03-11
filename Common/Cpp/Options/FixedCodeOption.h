/*  Fixed Code Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_FixedCodeOption_H
#define PokemonAutomation_Options_FixedCodeOption_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class FixedCodeOption : public ConfigOption{
public:
    ~FixedCodeOption();
    FixedCodeOption(
        std::string label,
        size_t digits,
        std::string default_value
    );
//    virtual std::unique_ptr<ConfigOption> clone() const override;

    const std::string& label() const;
    size_t digits() const;

    operator const std::string&() const;
    const std::string& get() const;
    std::string set(std::string x);
    std::string to_str() const;

    virtual std::string check_validity() const override;
    std::string check_validity(const std::string& x) const;
    virtual void restore_defaults() override;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    struct Data;
    Pimpl<Data> m_data;
};



}
#endif
