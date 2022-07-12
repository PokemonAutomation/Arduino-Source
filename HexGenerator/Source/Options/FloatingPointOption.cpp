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
#include "Tools/Tools.h"
#include "FloatingPointOption.h"

namespace PokemonAutomation{


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
    , FloatingPointBaseOption(
        QString::fromStdString(SingleStatementOption::m_label),
        obj.get_double_throw(JSON_MIN_VALUE),
        obj.get_double_throw(JSON_MAX_VALUE),
        obj.get_double_throw(JSON_DEFAULT)
    )
{
    m_current = obj.get_double_throw(JSON_CURRENT);
}
QString FloatingPoint::check_validity() const{
    return FloatingPointBaseOption::check_validity();
}
void FloatingPoint::restore_defaults(){
    FloatingPointBaseOption::restore_defaults();
}
JsonObject FloatingPoint::to_json() const{
    JsonObject root = SingleStatementOption::to_json();
    root[JSON_MIN_VALUE] = m_min_value;
    root[JSON_MAX_VALUE] = m_max_value;
    root[JSON_DEFAULT] = write_default();
    root[JSON_CURRENT] = write_current();
    return root;
}
std::string FloatingPoint::to_cpp() const{
    std::string str;
    str += m_declaration;
    str += " = ";
    str += std::to_string(m_current);
    str += ";\r\n";
    return str;
}
QWidget* FloatingPoint::make_ui(QWidget& parent){
    return new FloatingPointUI(parent, *this);
}

FloatingPointUI::FloatingPointUI(QWidget& parent, FloatingPoint& value)
    : FloatingPointBaseWidget(parent, value)
{}


}

