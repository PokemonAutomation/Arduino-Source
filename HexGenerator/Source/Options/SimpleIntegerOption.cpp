/*  Simple Integer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QIntValidator>
#include <QLineEdit>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonTools.h"
#include "Tools/Tools.h"
#include "SimpleIntegerOption.h"

namespace PokemonAutomation{


const std::string SimpleInteger::OPTION_TYPE = "SimpleInteger";
const std::string SimpleInteger::JSON_MIN_VALUE = "03-MinValue";
const std::string SimpleInteger::JSON_MAX_VALUE = "04-MaxValue";


int SimpleInteger_init = register_option(
    SimpleInteger::OPTION_TYPE,
        [](const JsonObject& obj){
        return std::unique_ptr<ConfigItem>(
            new SimpleInteger(obj)
        );
    }
);

SimpleInteger::SimpleInteger(const JsonObject& obj)
    : SingleStatementOption(obj)
    , SimpleIntegerBaseOption<uint32_t>(
        QString::fromStdString(SingleStatementOption::m_label),
        obj.get_integer_throw(JSON_MIN_VALUE),
        obj.get_integer_throw(JSON_MAX_VALUE),
        obj.get_integer_throw(JSON_DEFAULT)
    )
{
    m_current = obj.get_integer_throw(JSON_CURRENT);
}
QString SimpleInteger::check_validity() const{
    return SimpleIntegerBaseOption<uint32_t>::check_validity();
}
void SimpleInteger::restore_defaults(){
    SimpleIntegerBaseOption<uint32_t>::restore_defaults();
}
JsonObject SimpleInteger::to_json() const{
    JsonObject root = SingleStatementOption::to_json();
    root[JSON_MIN_VALUE] = m_min_value;
    root[JSON_MAX_VALUE] = m_max_value;
    root[JSON_DEFAULT] = write_default();
    root[JSON_CURRENT] = write_current();
    return root;
}
std::string SimpleInteger::to_cpp() const{
    std::string str;
    str += m_declaration;
    str += " = ";
    str += std::to_string(m_current);
    str += ";\r\n";
    return str;
}
QWidget* SimpleInteger::make_ui(QWidget& parent){
    return new SimpleIntegerUI(parent, *this);
}

SimpleIntegerUI::SimpleIntegerUI(QWidget& parent, SimpleInteger& value)
    : SimpleIntegerBaseWidget<uint32_t>(parent, value)
{}




}





