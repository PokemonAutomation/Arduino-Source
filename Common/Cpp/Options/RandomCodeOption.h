/*  Random Code Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_RandomCodeOption_H
#define PokemonAutomation_Options_RandomCodeOption_H

#include "Common/Cpp/SpinLock.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class RaidCodeOption{
public:
    RaidCodeOption(size_t total_digits);
    RaidCodeOption(size_t total_digits, size_t random_digits, std::string code_string);

    size_t total_digits() const{ return m_digits; }
    size_t random_digits() const{ return m_random_digits; }
    const std::string& code_string() const{ return m_code; }

    std::string check_validity() const;
    bool code_enabled() const;
    bool get_code(uint8_t* code) const;

//private:
    size_t m_digits;
    size_t m_random_digits;
    std::string m_code;
};



class RandomCodeOption : public ConfigOption{
public:
    RandomCodeOption(size_t total_digits);
    RandomCodeOption(std::string label, size_t total_digits, size_t random_digits, std::string code_string);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    operator RaidCodeOption() const;
    std::string set(RaidCodeOption code);

    bool code_enabled() const;
    bool get_code(uint8_t* code) const;

    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class RandomCodeWidget;
    const std::string m_label;
    const RaidCodeOption m_default;

    mutable SpinLock m_lock;
    RaidCodeOption m_current;
};






}
#endif
