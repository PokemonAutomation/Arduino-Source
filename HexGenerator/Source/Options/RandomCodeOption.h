/*  RandomCode
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_RandomCode_H
#define PokemonAutomation_RandomCode_H

#include <QLabel>
#include "ConfigItem.h"

namespace PokemonAutomation{


class RandomCode : public ConfigItem{
public:
    static const std::string OPTION_TYPE;
    static const std::string JSON_DECLARATION_RANDOM;
    static const std::string JSON_DECLARATION_CODE;
    static const std::string JSON_DIGITS;
    static const std::string JSON_DEFAULT_RANDOM;
    static const std::string JSON_DEFAULT_CODE;
    static const std::string JSON_CURRENT_RANDOM;
    static const std::string JSON_CURRENT_CODE;

public:
    RandomCode(const JsonObject& obj);

    virtual const std::string& type() const override{ return OPTION_TYPE; }
    virtual QString check_validity() const override;
    virtual void restore_defaults() override;

    virtual JsonObject to_json() const override;
    virtual std::string to_cpp() const override;

    virtual QWidget* make_ui(QWidget& parent) override;

private:
    friend class RandomCodeUI;
    std::string m_declaration_random;
    std::string m_declaration_code;
    size_t m_digits;
    size_t m_default_random;
    std::string m_default_code;
    size_t m_current_random;
    std::string m_current_code;
};



class RandomCodeUI : public QWidget{
public:
    RandomCodeUI(QWidget& parent, RandomCode& value, const std::string& label);
    virtual ~RandomCodeUI();

private:
    std::string sanitized_code(const std::string& text) const;
    std::string random_code_string() const;
    void update_labels();

private:
    RandomCode& m_value;
    QLabel* m_label_code;
    QLabel* m_under_text;
};


}
#endif
