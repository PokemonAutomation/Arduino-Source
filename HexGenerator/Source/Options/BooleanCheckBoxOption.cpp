/*  BooleanCheckBox
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonObject>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include "Common/Qt/QtJsonTools.h"
#include "Tools/Tools.h"
#include "BooleanCheckBoxOption.h"

namespace PokemonAutomation{


const QString BooleanCheckBox::OPTION_TYPE = "BooleanCheckBox";

int BooleanCheckBox_init = register_option(
    BooleanCheckBox::OPTION_TYPE,
        [](const QJsonObject& obj){
        return std::unique_ptr<ConfigItem>(
            new BooleanCheckBox(obj)
        );
    }
);


BooleanCheckBox::BooleanCheckBox(const QJsonObject& obj)
    : SingleStatementOption(obj)
    , BooleanCheckBoxBaseOption(SingleStatementOption::m_label, false)
{
    load_default(json_get_value_throw(obj, JSON_DEFAULT));
    load_current(json_get_value_throw(obj, JSON_CURRENT));
}
void BooleanCheckBox::restore_defaults(){
    BooleanCheckBoxBaseOption::restore_defaults();
}
QJsonObject BooleanCheckBox::to_json() const{
    QJsonObject root = SingleStatementOption::to_json();
    root.insert(JSON_DEFAULT, write_default());
    root.insert(JSON_CURRENT, write_current());
    return root;
}
std::string BooleanCheckBox::to_cpp() const{
    std::string str;
    str += m_declaration.toUtf8().data();
    str += " = ";
    str += *this ? "true" : "false";
    str += ";\r\n";
    return str;
}
QWidget* BooleanCheckBox::make_ui(QWidget& parent){
    return new BooleanCheckBoxUI(parent, *this);
}


BooleanCheckBoxUI::BooleanCheckBoxUI(QWidget& parent, BooleanCheckBox& value)
    : BooleanCheckBoxBaseWidget(parent, value)
{}


}
