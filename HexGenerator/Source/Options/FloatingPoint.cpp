/*  Floating Point
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleValidator>
#include <QLineEdit>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonTools.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "Tools/Tools.h"
#include "FloatingPoint.h"

namespace PokemonAutomation{
namespace HexGenerator{


const std::string FloatingPoint::OPTION_TYPE = "FloatingPoint";
const std::string FloatingPoint::JSON_MIN_VALUE = "03-MinValue";
const std::string FloatingPoint::JSON_MAX_VALUE = "04-MaxValue";


int FloatingPoint_init = register_option(
    FloatingPoint::OPTION_TYPE,
        [](const JsonObject& obj){
        return std::unique_ptr<ConfigItem>(
            new FloatingPoint(obj)
        );
    }
);

FloatingPoint::FloatingPoint(const JsonObject& obj)
    : SingleStatementOption(obj)
    , m_option(
        SingleStatementOption::m_label,
        LockMode::LOCK_WHILE_RUNNING,
        obj.get_double_throw(JSON_DEFAULT),
        obj.get_double_throw(JSON_MIN_VALUE),
        obj.get_double_throw(JSON_MAX_VALUE)
    )
{
    m_option.set(obj.get_double_throw(JSON_CURRENT));
}
std::string FloatingPoint::check_validity() const{
    return m_option.check_validity();
}
void FloatingPoint::restore_defaults(){
    m_option.restore_defaults();
}
JsonObject FloatingPoint::to_json() const{
    JsonObject root = SingleStatementOption::to_json();
    root[JSON_MIN_VALUE] = m_option.min_value();
    root[JSON_MAX_VALUE] = m_option.max_value();
    root[JSON_DEFAULT] = m_option.default_value();
    root[JSON_CURRENT] = (double)m_option;
    return root;
}
std::string FloatingPoint::to_cpp() const{
    std::string str;
    str += m_declaration;
    str += " = ";
    str += std::to_string(m_option);
    str += ";\r\n";
    return str;
}
QWidget* FloatingPoint::make_ui(QWidget& parent){
    return &m_option.make_QtWidget(parent)->widget();
}




}
}

