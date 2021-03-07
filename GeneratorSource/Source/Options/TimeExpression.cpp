/*  Time Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonObject>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "Common/Clientside/PrettyPrint.h"
#include "Common/Qt/StringException.h"
#include "Common/Qt/QtJsonTools.h"
#include "Common/Qt/ExpressionEvaluator.h"
#include "Tools/Tools.h"
#include "TimeExpression.h"

namespace PokemonAutomation{


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
    , TimeExpressionOption<uint32_t>(
        SingleStatementOption::m_label,
        json_get_int(obj, JSON_MIN_VALUE),
        json_get_int(obj, JSON_MAX_VALUE),
        json_get_string(obj, JSON_DEFAULT)
    )
{
    load_current(json_get_string(obj, JSON_CURRENT));
    update();
}

bool TimeExpression::is_valid() const{
    return TimeExpressionOption<uint32_t>::is_valid();
}
void TimeExpression::restore_defaults(){
    TimeExpressionOption<uint32_t>::restore_defaults();
}
QJsonObject TimeExpression::to_json() const{
    QJsonObject root = SingleStatementOption::to_json();
    root.insert(JSON_MIN_VALUE, QJsonValue((qint64)min()));
    root.insert(JSON_MAX_VALUE, QJsonValue((qint64)max()));
    root.insert(JSON_DEFAULT, write_default());
    root.insert(JSON_CURRENT, write_current());
    return root;
}
std::string TimeExpression::to_cpp() const{
    std::string str;
    str += m_declaration.toUtf8().data();
    str += " = ";
    str += std::to_string(value());
    str += ";\r\n";
    return str;
}
QWidget* TimeExpression::make_ui(QWidget& parent){
    return new TimeExpressionOptionUI<uint32_t>(parent, *this);
}

TimeExpressionUI::TimeExpressionUI(QWidget& parent, TimeExpression& value)
    : TimeExpressionOptionUI(parent, value)
{}



}
