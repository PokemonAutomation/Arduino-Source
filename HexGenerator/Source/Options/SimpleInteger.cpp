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
#include "SimpleInteger.h"

namespace PokemonAutomation{
namespace HexGenerator{


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
    , m_option(
        SingleStatementOption::m_label,
        obj.get_integer_throw(JSON_DEFAULT),
        obj.get_integer_throw(JSON_MIN_VALUE),
        obj.get_integer_throw(JSON_MAX_VALUE)
    )
{
    m_option.set((uint32_t)obj.get_integer_throw(JSON_CURRENT));
}
std::string SimpleInteger::check_validity() const{
    return m_option.check_validity();
}
void SimpleInteger::restore_defaults(){
    m_option.restore_defaults();
}
JsonObject SimpleInteger::to_json() const{
    JsonObject root = SingleStatementOption::to_json();
    root[JSON_MIN_VALUE] = m_option.min_value();
    root[JSON_MAX_VALUE] = m_option.max_value();
    root[JSON_DEFAULT] = m_option.default_value();
    root[JSON_CURRENT] = (uint32_t)m_option;
    return root;
}
std::string SimpleInteger::to_cpp() const{
    std::string str;
    str += m_declaration;
    str += " = ";
    str += std::to_string(m_option);
    str += ";\r\n";
    return str;
}
QWidget* SimpleInteger::make_ui(QWidget& parent){
    return &m_option.make_ui(parent)->widget();
}





}
}





