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
#include "FossilTableOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


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
    , m_table(SingleStatementOption::m_label, m_factory, true)
{
    m_table.load_default(json_get_array_throw(obj, JSON_DEFAULT));
    m_table.load_current(json_get_array_throw(obj, JSON_CURRENT));
}
QString FossilTable::check_validity() const{
    return m_table.check_validity();
}
void FossilTable::restore_defaults(){
    m_table.restore_defaults();
}
QJsonObject FossilTable::to_json() const{
    QJsonObject root = SingleStatementOption::to_json();
    root.insert(JSON_DEFAULT, m_table.write_default());
    root.insert(JSON_CURRENT, m_table.write_current());
    return root;
}
std::string FossilTable::to_cpp() const{
    std::string str;
    str += m_declaration.toUtf8().data();
    str += " = {\r\n";
    for (size_t c = 0; c < m_table.size(); c++){
        const FossilGame& item = static_cast<const FossilGame&>(m_table[c]);
        str += "    {";
        str += std::to_string(item.game_slot);
        str += ", ";
        str += std::to_string(item.user_slot);
        str += ", ";
        str += FossilGame::FOSSIL_LIST[item.fossil].toUtf8().data();
        str += ", ";
        str += std::to_string(item.revives);
        str += "},\r\n";
    }
    str += "    {},\r\n";
    str += "};\r\n";
    return str;
}
QWidget* FossilTable::make_ui(QWidget& parent){
    return new EditableTableBaseWidget(parent, m_table);
}






}
}
}


