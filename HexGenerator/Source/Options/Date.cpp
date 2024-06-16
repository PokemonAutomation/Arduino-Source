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
#include "Common/Qt/Options/ConfigWidget.h"
#include "Tools/Tools.h"
#include "Date.h"

namespace PokemonAutomation{
namespace HexGenerator{


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
    , m_option(
        SingleStatementOption::m_label,
        LockMode::LOCK_WHILE_RUNNING,
        DateTimeCell::Level::DATE,
        DateTime{2000, 1, 1}, DateTime{2060, 12, 31},
        DateTimeOption::from_json(obj.get_value_throw(JSON_DEFAULT))
    )
{
    m_option.set(DateTimeOption::from_json(obj.get_value_throw(JSON_CURRENT)));
}
std::string SwitchDate::check_validity() const{
    return m_option.check_validity();
}
void SwitchDate::restore_defaults(){
    m_option.restore_defaults();
}
JsonObject SwitchDate::to_json() const{
    JsonObject root = SingleStatementOption::to_json();
    root[JSON_DEFAULT] = DateTimeOption::to_json(m_option.default_value());
    root[JSON_CURRENT] = m_option.to_json();
    return root;
}
std::string SwitchDate::to_cpp() const{
    DateTime date = m_option;

    std::string str;
    str += m_declaration;
    str += " = {";
    str += std::to_string(date.year);
    str += ", ";
    str += std::to_string(date.month);
    str += ", ";
    str += std::to_string(date.day);
    str += "};\r\n";
    return str;
}
QWidget* SwitchDate::make_ui(QWidget& parent){
    return &m_option.make_QtWidget(parent)->widget();
}




}
}



