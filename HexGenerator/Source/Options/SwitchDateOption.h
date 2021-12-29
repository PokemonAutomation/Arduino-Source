/*  Switch Date
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SwitchDate_H
#define PokemonAutomation_SwitchDate_H

#include <QDate>
#include "Common/Qt/Options/SwitchDate/SwitchDateBaseOption.h"
#include "Common/Qt/Options/SwitchDate/SwitchDateBaseWidget.h"
#include "SingleStatementOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SwitchDate : public SingleStatementOption, public SwitchDateBaseOption{
public:
    static const QString OPTION_TYPE;

public:
    SwitchDate(const QJsonObject& obj);

    virtual const QString& type() const override{ return OPTION_TYPE; }
    virtual QString check_validity() const override;
    virtual void restore_defaults() override;

    virtual QJsonObject to_json() const override;
    virtual std::string to_cpp() const override;

    virtual QWidget* make_ui(QWidget& parent) override;

private:
    friend class SwitchDateUI;
};

class SwitchDateUI : public SwitchDateBaseWidget{
public:
    SwitchDateUI(QWidget& parent, SwitchDate& value);
};


}
}
#endif


