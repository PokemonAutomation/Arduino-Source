/*  Random Code Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_RandomCode_H
#define PokemonAutomation_RandomCode_H

#include <QString>
#include "ConfigOption.h"

namespace PokemonAutomation{


class RaidCodeOption{
public:
    RaidCodeOption();
    RaidCodeOption(size_t random_digits, QString code_string);

    size_t total_digits() const{ return m_digits; }
    size_t random_digits() const{ return m_random_digits; }
    const QString& code_string() const{ return m_code; }

    QString check_validity() const;
    bool code_enabled() const;
    bool get_code(uint8_t* code) const;

//private:
    size_t m_digits;
    size_t m_random_digits;
    QString m_code;
};

class RandomCodeOption : public ConfigOption{
public:
    RandomCodeOption();
    RandomCodeOption(QString label, size_t random_digits, QString code_string);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    bool code_enabled() const;
    size_t random_digits() const{ return m_current.random_digits(); }
    bool get_code(uint8_t* code) const;

    virtual QString check_validity() const override;
    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class RandomCodeWidget;
    QString m_label;
    const RaidCodeOption m_default;
    RaidCodeOption m_current;
};






}
#endif
