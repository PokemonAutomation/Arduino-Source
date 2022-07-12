/*  Switch Date
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QDateEdit>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonTools.h"
#include "Common/Qt/QtJsonTools.h"
#include "Tools/Tools.h"
#include "SwitchDateOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


const std::string SwitchDate::OPTION_TYPE = "SwitchDate";


int SwitchDate_init = register_option(
    SwitchDate::OPTION_TYPE,
        [](const JsonObject& obj){
        return std::unique_ptr<ConfigItem>(
            new SwitchDate(obj)
        );
    }
);

SwitchDate::SwitchDate(const JsonObject& obj)
    : SingleStatementOption(obj)
    , SwitchDateBaseOption(QString::fromStdString(SingleStatementOption::m_label), QDate(2000, 1, 1))
{
    load_default(obj.get_value_throw(JSON_DEFAULT));
    load_current(obj.get_value_throw(JSON_CURRENT));
}
QString SwitchDate::check_validity() const{
    return SwitchDateBaseOption::check_validity();
}
void SwitchDate::restore_defaults(){
    SwitchDateBaseOption::restore_defaults();
}
JsonObject SwitchDate::to_json() const{
    JsonObject root = SingleStatementOption::to_json();
    root[JSON_DEFAULT] = write_default();
    root[JSON_CURRENT] = write_current();
    return root;
}
std::string SwitchDate::to_cpp() const{
    std::string str;
    str += m_declaration;
    str += " = {";
    str += std::to_string(m_current.year());
    str += ", ";
    str += std::to_string(m_current.month());
    str += ", ";
    str += std::to_string(m_current.day());
    str += "};\r\n";
    return str;
}
QWidget* SwitchDate::make_ui(QWidget& parent){
    return new SwitchDateUI(parent, *this);
}

SwitchDateUI::SwitchDateUI(QWidget& parent, SwitchDate& value)
    : SwitchDateBaseWidget(parent, value)
{}



}
}



