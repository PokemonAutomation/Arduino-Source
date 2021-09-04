/*  Fossil Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include <QJsonArray>
#include <QVBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include "Common/Qt/QtJsonTools.h"
#include "Tools/Tools.h"
#include "FossilTable.h"

namespace PokemonAutomation{


const QString FossilTable::OPTION_TYPE      = "FossilTable";


int FossilTable_init = register_option(
    FossilTable::OPTION_TYPE,
        [](const QJsonObject& obj){
        return std::unique_ptr<ConfigItem>(
            new FossilTable(obj)
        );
    }
);


FossilTable::FossilTable(const QJsonObject& obj)
    : SingleStatementOption(obj)
    , FossilTableOption(SingleStatementOption::m_label)
{
    load_default(json_get_array_throw(obj, JSON_DEFAULT));
    load_current(json_get_array_throw(obj, JSON_CURRENT));
}
bool FossilTable::is_valid() const{
    return FossilTableOption::is_valid();
}
void FossilTable::restore_defaults(){
    FossilTableOption::restore_defaults();
}
QJsonObject FossilTable::to_json() const{
    QJsonObject root = SingleStatementOption::to_json();
    root.insert(JSON_DEFAULT, write_default());
    root.insert(JSON_CURRENT, write_current());
    return root;
}
std::string FossilTable::to_cpp() const{
    std::string str;
    str += m_declaration.toUtf8().data();
    str += " = {\r\n";
    for (const auto& item : m_current){
        str += "    {";
        str += std::to_string(item.game_slot);
        str += ", ";
        str += std::to_string(item.user_slot);
        str += ", ";
        str += FOSSIL_LIST[item.fossil].toUtf8().data();
        str += ", ";
        str += std::to_string(item.revives);
        str += "},\r\n";
    }
    str += "    {},\r\n";
    str += "};\r\n";
    return str;
}
QWidget* FossilTable::make_ui(QWidget& parent){
    return new FossilTableUI(parent, *this);
}


FossilTableUI::FossilTableUI(QWidget& parent, FossilTable& value)
    : FossilTableOptionUI(parent, value)
{}





}


