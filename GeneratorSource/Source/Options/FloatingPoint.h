/*  Floating Point
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 *
 */

#ifndef PokemonAutomation_SimpleInteger_H
#define PokemonAutomation_SimpleInteger_H

#include "Common/Qt/Options/FloatingPointOption.h"
#include "SingleStatementOption.h"

namespace PokemonAutomation{



class FloatingPoint : public SingleStatementOption, public FloatingPointOption{
public:
    static const QString OPTION_TYPE;
    static const QString JSON_MIN_VALUE;
    static const QString JSON_MAX_VALUE;

public:
    FloatingPoint(const QJsonObject& obj);

    virtual const QString& type() const override{ return OPTION_TYPE; }
    virtual bool is_valid() const override;
    virtual void restore_defaults() override;

    virtual QJsonObject to_json() const override;
    virtual std::string to_cpp() const override;

    virtual QWidget* make_ui(QWidget& parent) override;

private:
    friend class FloatingPointUI;
};

class FloatingPointUI : public FloatingPointOptionUI{
public:
    FloatingPointUI(QWidget& parent, FloatingPoint& value);
};


}
#endif


