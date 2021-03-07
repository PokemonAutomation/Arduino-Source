/*  Time Option
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 *
 */

#ifndef PokemonAutomation_TimeOption_H
#define PokemonAutomation_TimeOption_H

#include "Common/Qt/Options/TimeExpressionOption.h"
#include "SingleStatementOption.h"

namespace PokemonAutomation{


class TimeExpression : public SingleStatementOption, public TimeExpressionOption<uint32_t>{
public:
    static const QString OPTION_TYPE;
    static const QString JSON_MIN_VALUE;
    static const QString JSON_MAX_VALUE;

public:
    TimeExpression(const QJsonObject& obj);

    virtual const QString& type() const override{ return OPTION_TYPE; }
    bool is_valid() const override;
    virtual void restore_defaults() override;

    virtual QJsonObject to_json() const override;
    virtual std::string to_cpp() const override;

    virtual QWidget* make_ui(QWidget& parent) override;
};


class TimeExpressionUI : public TimeExpressionOptionUI<uint32_t>{
public:
    TimeExpressionUI(QWidget& parent, TimeExpression& value);
};


}
#endif


