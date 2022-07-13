/*  Floating Point
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SimpleInteger_H
#define PokemonAutomation_SimpleInteger_H

#include "Common/Qt/Options/FloatingPoint/FloatingPointBaseOption.h"
#include "Common/Qt/Options/FloatingPoint/FloatingPointBaseWidget.h"
#include "SingleStatementOption.h"

namespace PokemonAutomation{



class FloatingPoint : public SingleStatementOption, public FloatingPointBaseOption{
public:
    static const std::string OPTION_TYPE;
    static const std::string JSON_MIN_VALUE;
    static const std::string JSON_MAX_VALUE;

public:
    FloatingPoint(const JsonObject& obj);

    virtual const std::string& type() const override{ return OPTION_TYPE; }
    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;

    virtual JsonObject to_json() const override;
    virtual std::string to_cpp() const override;

    virtual QWidget* make_ui(QWidget& parent) override;

private:
    friend class FloatingPointUI;
};

class FloatingPointUI : public FloatingPointBaseWidget{
public:
    FloatingPointUI(QWidget& parent, FloatingPoint& value);
};


}
#endif


