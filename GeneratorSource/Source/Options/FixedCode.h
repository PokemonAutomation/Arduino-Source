/*  FixedCode
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 *
 */

#ifndef PokemonAutomation_FixedCode_H
#define PokemonAutomation_FixedCode_H

#include "SingleStatementOption.h"

namespace PokemonAutomation{


class FixedCode : public SingleStatementOption{
public:
    static const QString OPTION_TYPE;
    static const QString JSON_DIGITS;

public:
    FixedCode(const QJsonObject& obj);

    virtual const QString& type() const override{ return OPTION_TYPE; }
    virtual bool is_valid() const override;
    virtual void restore_defaults() override;

    virtual QJsonObject to_json() const override;
    virtual std::string to_cpp() const override;

    virtual QWidget* make_ui(QWidget& parent) override;

private:
    friend class FixedCodeUI;
    size_t m_digits;
    QString m_default;
    QString m_current;
};



class FixedCodeUI : public QWidget{
public:
    FixedCodeUI(QWidget& parent, FixedCode& value, const QString& label);
    ~FixedCodeUI();

private:
    QString sanitized_code(const QString& text);

private:
    FixedCode& m_value;
};



}
#endif
