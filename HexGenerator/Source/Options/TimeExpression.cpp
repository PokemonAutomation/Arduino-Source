/*  Time Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonTools.h"
#include "Common/Qt/ExpressionEvaluator.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "Tools/Tools.h"
#include "TimeExpression.h"

namespace PokemonAutomation{
namespace HexGenerator{


const std::string TimeExpression::OPTION_TYPE = "TimeExpression";
const std::string TimeExpression::JSON_MIN_VALUE = "03-MinValue";
const std::string TimeExpression::JSON_MAX_VALUE = "04-MaxValue";


int TimeExpression_init = register_option(
    TimeExpression::OPTION_TYPE,
        [](const JsonObject& obj){
        return std::unique_ptr<ConfigItem>(
            new TimeExpression(obj)
        );
    }
);


TimeExpression::TimeExpression(const JsonObject& obj)
    : SingleStatementOption(obj)
    , m_option(
        TICKS_PER_SECOND,
        SingleStatementOption::m_label,
        obj.get_string_throw(JSON_DEFAULT),
        obj.get_integer_throw(JSON_MIN_VALUE),
        obj.get_integer_throw(JSON_MAX_VALUE)
    )
{
    m_option.set(obj.get_string_throw(JSON_CURRENT));
}

std::string TimeExpression::check_validity() const{
    return m_option.check_validity();
}
void TimeExpression::restore_defaults(){
    m_option.restore_defaults();
}
JsonObject TimeExpression::to_json() const{
    JsonObject root = SingleStatementOption::to_json();
    root[JSON_MIN_VALUE] = m_option.min_value();
    root[JSON_MAX_VALUE] = m_option.max_value();
    root[JSON_DEFAULT] = m_option.default_value();
    root[JSON_CURRENT] = m_option.text();
    return root;
}
std::string TimeExpression::to_cpp() const{
    std::string str;
    str += m_declaration;
    str += " = ";
    str += std::to_string(m_option.get());
    str += ";\r\n";
    return str;
}
QWidget* TimeExpression::make_ui(QWidget& parent){
    return &m_option.make_ui(parent)->widget();
}




}
}
