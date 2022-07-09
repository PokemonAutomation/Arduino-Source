/*  Time Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonObject>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonTools.h"
#include "Common/Qt/QtJsonTools.h"
#include "Common/Qt/ExpressionEvaluator.h"
#include "Tools/Tools.h"
#include "TimeExpressionOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


const QString TimeExpression::OPTION_TYPE = "TimeExpression";
const QString TimeExpression::JSON_MIN_VALUE = "03-MinValue";
const QString TimeExpression::JSON_MAX_VALUE = "04-MaxValue";


int TimeExpression_init = register_option(
    TimeExpression::OPTION_TYPE,
        [](const QJsonObject& obj){
        return std::unique_ptr<ConfigItem>(
            new TimeExpression(obj)
        );
    }
);


TimeExpression::TimeExpression(const QJsonObject& obj)
    : SingleStatementOption(obj)
    , TimeExpressionBaseOption<uint32_t>(
        SingleStatementOption::m_label,
        json_get_int_throw(obj, JSON_MIN_VALUE),
        json_get_int_throw(obj, JSON_MAX_VALUE),
        json_get_string_throw(obj, JSON_DEFAULT)
    )
{
    load_current(from_QJson(json_get_string_throw(obj, JSON_CURRENT)));
}

QString TimeExpression::check_validity() const{
    return TimeExpressionBaseOption<uint32_t>::check_validity();
}
void TimeExpression::restore_defaults(){
    TimeExpressionBaseOption<uint32_t>::restore_defaults();
}
QJsonObject TimeExpression::to_json() const{
    QJsonObject root = SingleStatementOption::to_json();
    root.insert(JSON_MIN_VALUE, QJsonValue((qint64)min()));
    root.insert(JSON_MAX_VALUE, QJsonValue((qint64)max()));
    root.insert(JSON_DEFAULT, to_QJson(write_default()));
    root.insert(JSON_CURRENT, to_QJson(write_current()));
    return root;
}
std::string TimeExpression::to_cpp() const{
    std::string str;
    str += m_declaration.toUtf8().data();
    str += " = ";
    str += std::to_string(get());
    str += ";\r\n";
    return str;
}
QWidget* TimeExpression::make_ui(QWidget& parent){
    return new TimeExpressionBaseWidget<uint32_t>(parent, *this);
}

TimeExpressionUI::TimeExpressionUI(QWidget& parent, TimeExpression& value)
    : TimeExpressionBaseWidget(parent, value)
{}



}
}
