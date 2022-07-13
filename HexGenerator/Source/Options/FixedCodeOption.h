/*  FixedCode
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_FixedCode_H
#define PokemonAutomation_FixedCode_H

#include "SingleStatementOption.h"

namespace PokemonAutomation{


class FixedCode : public SingleStatementOption{
public:
    static const std::string OPTION_TYPE;
    static const std::string JSON_DIGITS;

public:
    FixedCode(const JsonObject& obj);

    virtual const std::string& type() const override{ return OPTION_TYPE; }
    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;

    virtual JsonObject to_json() const override;
    virtual std::string to_cpp() const override;

    virtual QWidget* make_ui(QWidget& parent) override;

private:
    friend class FixedCodeUI;
    size_t m_digits;
    std::string m_default;
    std::string m_current;
};



class FixedCodeUI : public QWidget{
public:
    FixedCodeUI(QWidget& parent, FixedCode& value, const std::string& label);
    ~FixedCodeUI();

private:
    std::string sanitized_code(const std::string& text);

private:
    FixedCode& m_value;
};



}
#endif
