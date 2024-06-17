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
#include "Common/Qt/Options/ConfigWidget.h"
#include "Tools/Tools.h"
#include "BooleanCheckBox.h"

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
    , m_option(
        SingleStatementOption::m_label,
        LockMode::LOCK_WHILE_RUNNING,
        obj.get_value_throw(JSON_DEFAULT).to_boolean_throw()
    )
{
    m_option = obj.get_value_throw(JSON_CURRENT).to_boolean_throw();
}
void BooleanCheckBox::restore_defaults(){
    m_option.restore_defaults();
}
JsonObject BooleanCheckBox::to_json() const{
    JsonObject root = SingleStatementOption::to_json();
    root[JSON_DEFAULT] = m_option.default_value();
    root[JSON_CURRENT] = (bool)m_option;
    return root;
}
std::string BooleanCheckBox::to_cpp() const{
    std::string str;
    str += m_declaration;
    str += " = ";
    str += m_option ? "true" : "false";
    str += ";\r\n";
    return str;
}
QWidget* BooleanCheckBox::make_ui(QWidget& parent){
    return &m_option.make_QtWidget(parent)->widget();
}




}
}
