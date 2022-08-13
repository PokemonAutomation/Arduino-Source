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
#include "Tools/Tools.h"
#include "TimeExpressionOption.h"

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
    , TimeExpressionBaseOption<uint32_t>(
        SingleStatementOption::m_label,
        obj.get_integer_throw(JSON_MIN_VALUE),
        obj.get_integer_throw(JSON_MAX_VALUE),
        obj.get_string_throw(JSON_DEFAULT)
    )
{
    load_current(obj.get_string_throw(JSON_CURRENT));
}

std::string TimeExpression::check_validity() const{
    return TimeExpressionBaseOption<uint32_t>::check_validity();
}
void TimeExpression::restore_defaults(){
    TimeExpressionBaseOption<uint32_t>::restore_defaults();
}
JsonObject TimeExpression::to_json() const{
    JsonObject root = SingleStatementOption::to_json();
    root[JSON_MIN_VALUE] = min();
    root[JSON_MAX_VALUE] = max();
    root[JSON_DEFAULT] = write_default();
    root[JSON_CURRENT] = write_current();
    return root;
}
std::string TimeExpression::to_cpp() const{
    std::string str;
    str += m_declaration;
    str += " = ";
    str += std::to_string(get());
    str += ";\r\n";
    return str;
}
QWidget* TimeExpression::make_ui(QWidget& parent){
    return new NintendoSwitch::TimeExpressionBaseWidget<uint32_t>(parent, *this);
}

TimeExpressionUI::TimeExpressionUI(QWidget& parent, TimeExpression& value)
    : TimeExpressionBaseWidget(parent, value)
{}



}
}
