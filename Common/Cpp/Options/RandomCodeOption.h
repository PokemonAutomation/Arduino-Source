/*  Random Code Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_RandomCodeOption_H
#define PokemonAutomation_Options_RandomCodeOption_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class RaidCodeOption{
public:
    ~RaidCodeOption();
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
//    virtual std::unique_ptr<ConfigOption> clone() const override;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    const std::string& label() const;

    operator RaidCodeOption() const;
    std::string set(RaidCodeOption code);

    bool code_enabled() const;
    bool get_code(uint8_t* code) const;

    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    struct Data;
    Pimpl<Data> m_data;
};






}
#endif
