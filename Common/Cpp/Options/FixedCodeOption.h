/*  Fixed Code Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_FixedCodeOption_H
#define PokemonAutomation_Options_FixedCodeOption_H

#include "Common/Cpp/Concurrency/SpinLock.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class FixedCodeOption : public ConfigOption{
public:
    FixedCodeOption(
        std::string label,
        size_t digits,
        std::string default_value
    );
//    virtual std::unique_ptr<ConfigOption> clone() const override;

    operator const std::string&() const;
    const std::string& get() const;
    std::string set(std::string x);
    void to_str(uint8_t* code) const;

    virtual std::string check_validity() const override;
    std::string check_validity(const std::string& x) const;
    virtual void restore_defaults() override;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    friend class FixedCodeWidget;
    std::string m_label;
    const size_t m_digits;
    const std::string m_default;

    mutable SpinLock m_lock;
    std::string m_current;
};



}
#endif
