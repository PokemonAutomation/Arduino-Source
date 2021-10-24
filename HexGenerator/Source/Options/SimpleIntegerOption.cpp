/*  Simple Integer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonObject>
#include <QHBoxLayout>
#include <QLabel>
#include <QIntValidator>
#include <QLineEdit>
#include "Common/Qt/QtJsonTools.h"
#include "Tools/Tools.h"
#include "SimpleIntegerOption.h"

namespace PokemonAutomation{


const QString SimpleInteger::OPTION_TYPE = "SimpleInteger";
const QString SimpleInteger::JSON_MIN_VALUE = "03-MinValue";
const QString SimpleInteger::JSON_MAX_VALUE = "04-MaxValue";


int SimpleInteger_init = register_option(
    SimpleInteger::OPTION_TYPE,
        [](const QJsonObject& obj){
        return std::unique_ptr<ConfigItem>(
            new SimpleInteger(obj)
        );
    }
);

SimpleInteger::SimpleInteger(const QJsonObject& obj)
    : SingleStatementOption(obj)
    , SimpleIntegerOptionBase<uint32_t>(
        SingleStatementOption::m_label,
        json_get_int_throw(obj, JSON_MIN_VALUE),
        json_get_int_throw(obj, JSON_MAX_VALUE),
        json_get_int_throw(obj, JSON_DEFAULT)
    )
{
    m_current = json_get_int_throw(obj, JSON_CURRENT);
}
QString SimpleInteger::check_validity() const{
    return SimpleIntegerOptionBase<uint32_t>::check_validity();
}
void SimpleInteger::restore_defaults(){
    SimpleIntegerOptionBase<uint32_t>::restore_defaults();
}
QJsonObject SimpleInteger::to_json() const{
    QJsonObject root = SingleStatementOption::to_json();
    root.insert(JSON_MIN_VALUE, QJsonValue((qint64)m_min_value));
    root.insert(JSON_MAX_VALUE, QJsonValue((qint64)m_max_value));
    root.insert(JSON_DEFAULT, write_default());
    root.insert(JSON_CURRENT, write_current());
    return root;
}
std::string SimpleInteger::to_cpp() const{
    std::string str;
    str += m_declaration.toUtf8().data();
    str += " = ";
    str += std::to_string(m_current);
    str += ";\r\n";
    return str;
}
QWidget* SimpleInteger::make_ui(QWidget& parent){
    return new SimpleIntegerUI(parent, *this);
}

SimpleIntegerUI::SimpleIntegerUI(QWidget& parent, SimpleInteger& value)
    : SimpleIntegerOptionBaseUI<uint32_t>(parent, value)
{}




}





