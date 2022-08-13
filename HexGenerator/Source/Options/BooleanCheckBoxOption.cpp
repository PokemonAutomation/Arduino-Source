/*  BooleanCheckBox
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonTools.h"
#include "Tools/Tools.h"
#include "BooleanCheckBoxOption.h"

namespace PokemonAutomation{
namespace HexGenerator{


const std::string BooleanCheckBox::OPTION_TYPE = "BooleanCheckBox";

int BooleanCheckBox_init = register_option(
    BooleanCheckBox::OPTION_TYPE,
        [](const JsonObject& obj){
        return std::unique_ptr<ConfigItem>(
            new BooleanCheckBox(obj)
        );
    }
);


BooleanCheckBox::BooleanCheckBox(const JsonObject& obj)
    : SingleStatementOption(obj)
    , BooleanCheckBoxBaseOption(SingleStatementOption::m_label, false)
{
    load_default(obj.get_value_throw(JSON_DEFAULT));
    load_current(obj.get_value_throw(JSON_CURRENT));
}
void BooleanCheckBox::restore_defaults(){
    BooleanCheckBoxBaseOption::restore_defaults();
}
JsonObject BooleanCheckBox::to_json() const{
    JsonObject root = SingleStatementOption::to_json();
    root[JSON_DEFAULT] = write_default();
    root[JSON_CURRENT] = write_current();
    return root;
}
std::string BooleanCheckBox::to_cpp() const{
    std::string str;
    str += m_declaration;
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
}
