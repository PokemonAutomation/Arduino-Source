/*  BooleanCheckBox
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_BooleanCheckBox_H
#define PokemonAutomation_BooleanCheckBox_H

#include "Common/Qt/Options/BooleanCheckBox/BooleanCheckBoxBaseOption.h"
#include "Common/Qt/Options/BooleanCheckBox/BooleanCheckBoxBaseWidget.h"
#include "SingleStatementOption.h"

namespace PokemonAutomation{


class BooleanCheckBox : public SingleStatementOption, private BooleanCheckBoxBaseOption{
public:
    static const QString OPTION_TYPE;

public:
    BooleanCheckBox(const QJsonObject& obj);

    virtual const QString& type() const override{ return OPTION_TYPE; }
    virtual void restore_defaults() override;

    virtual QJsonObject to_json() const override;
    virtual std::string to_cpp() const override;

    virtual QWidget* make_ui(QWidget& parent) override;

private:
    friend class BooleanCheckBoxUI;
};



class BooleanCheckBoxUI : public BooleanCheckBoxBaseWidget{
public:
    BooleanCheckBoxUI(QWidget& parent, BooleanCheckBox& value);
};


}
#endif
