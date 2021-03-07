/*  Switch Date
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonArray>
#include <QJsonObject>
#include <QHBoxLayout>
#include <QLabel>
#include <QDateEdit>
#include "Common/Qt/QtJsonTools.h"
#include "Tools/Tools.h"
#include "SwitchDate.h"

namespace PokemonAutomation{


const QString SwitchDate::OPTION_TYPE = "SwitchDate";


int SwitchDate_init = register_option(
    SwitchDate::OPTION_TYPE,
        [](const QJsonObject& obj){
        return std::unique_ptr<ConfigItem>(
            new SwitchDate(obj)
        );
    }
);

SwitchDate::SwitchDate(const QJsonObject& obj)
    : SingleStatementOption(obj)
    , SwitchDateOption(SingleStatementOption::m_label, QDate(2000, 1, 1))
{
    load_default(json_get_value(obj, JSON_DEFAULT));
    load_current(json_get_value(obj, JSON_CURRENT));
}
bool SwitchDate::is_valid() const{
    return SwitchDateOption::is_valid();
}
void SwitchDate::restore_defaults(){
    SwitchDateOption::restore_defaults();
}
QJsonObject SwitchDate::to_json() const{
    QJsonObject root = SingleStatementOption::to_json();
    root.insert(JSON_DEFAULT, write_default());
    root.insert(JSON_CURRENT, write_current());
    return root;
}
std::string SwitchDate::to_cpp() const{
    std::string str;
    str += m_declaration.toUtf8().data();
    str += " = {";
    str += std::to_string(m_current.year());
    str += ", ";
    str += std::to_string(m_current.month());
    str += ", ";
    str += std::to_string(m_current.day());
    str += "};\r\n";
    return str;
}
QWidget* SwitchDate::make_ui(QWidget& parent){
    return new SwitchDateUI(parent, *this);
}

SwitchDateUI::SwitchDateUI(QWidget& parent, SwitchDate& value)
    : SwitchDateOptionUI(parent, value)
{}



}



