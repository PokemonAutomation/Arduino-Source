/*  Floating Point
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonObject>
#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleValidator>
#include <QLineEdit>
#include "Common/Qt/QtJsonTools.h"
#include "Tools/Tools.h"
#include "FloatingPointOption.h"

namespace PokemonAutomation{


const QString FloatingPoint::OPTION_TYPE = "FloatingPoint";
const QString FloatingPoint::JSON_MIN_VALUE = "03-MinValue";
const QString FloatingPoint::JSON_MAX_VALUE = "04-MaxValue";


int FloatingPoint_init = register_option(
    FloatingPoint::OPTION_TYPE,
        [](const QJsonObject& obj){
        return std::unique_ptr<ConfigItem>(
            new FloatingPoint(obj)
        );
    }
);

FloatingPoint::FloatingPoint(const QJsonObject& obj)
    : SingleStatementOption(obj)
    , FloatingPointBaseOption(
        SingleStatementOption::m_label,
        json_get_double_throw(obj, JSON_MIN_VALUE),
        json_get_double_throw(obj, JSON_MAX_VALUE),
        json_get_double_throw(obj, JSON_DEFAULT)
    )
{
    m_current = json_get_double_throw(obj, JSON_CURRENT);
}
QString FloatingPoint::check_validity() const{
    return FloatingPointBaseOption::check_validity();
}
void FloatingPoint::restore_defaults(){
    FloatingPointBaseOption::restore_defaults();
}
QJsonObject FloatingPoint::to_json() const{
    QJsonObject root = SingleStatementOption::to_json();
    root.insert(JSON_MIN_VALUE, QJsonValue(m_min_value));
    root.insert(JSON_MAX_VALUE, QJsonValue(m_max_value));
    root.insert(JSON_DEFAULT, write_default());
    root.insert(JSON_CURRENT, write_current());
    return root;
}
std::string FloatingPoint::to_cpp() const{
    std::string str;
    str += m_declaration.toUtf8().data();
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

