/*  Floating Point
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SimpleInteger_H
#define PokemonAutomation_SimpleInteger_H

#include "Common/Qt/Options/FloatingPointOptionBase.h"
#include "SingleStatementOption.h"

namespace PokemonAutomation{



class FloatingPoint : public SingleStatementOption, public FloatingPointOptionBase{
public:
    static const QString OPTION_TYPE;
    static const QString JSON_MIN_VALUE;
    static const QString JSON_MAX_VALUE;

public:
    FloatingPoint(const QJsonObject& obj);

    virtual const QString& type() const override{ return OPTION_TYPE; }
    virtual QString check_validity() const override;
    virtual void restore_defaults() override;

    virtual QJsonObject to_json() const override;
    virtual std::string to_cpp() const override;

    virtual QWidget* make_ui(QWidget& parent) override;

private:
    friend class FloatingPointUI;
};

class FloatingPointUI : public FloatingPointOptionBaseUI{
public:
    FloatingPointUI(QWidget& parent, FloatingPoint& value);
};


}
#endif


