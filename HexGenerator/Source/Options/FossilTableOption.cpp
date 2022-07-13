/*  Fossil Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include <QVBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonTools.h"
#include "Common/Qt/Options/EditableTable/EditableTableBaseWidget.h"
#include "Tools/Tools.h"
#include "FossilTableOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


const std::string FossilTable::OPTION_TYPE      = "FossilTable";


int FossilTable_init = register_option(
    FossilTable::OPTION_TYPE,
        [](const JsonObject& obj){
        return std::unique_ptr<ConfigItem>(
            new FossilTable(obj)
        );
    }
);


FossilTable::FossilTable(const JsonObject& obj)
    : SingleStatementOption(obj)
    , m_table(SingleStatementOption::m_label, m_factory)
{
    m_table.load_default(obj.get_value_throw(JSON_DEFAULT));
    m_table.load_current(obj.get_value_throw(JSON_CURRENT));
}
std::string FossilTable::check_validity() const{
    return m_table.check_validity();
}
void FossilTable::restore_defaults(){
    m_table.restore_defaults();
}
JsonObject FossilTable::to_json() const{
    JsonObject root = SingleStatementOption::to_json();
    root[JSON_DEFAULT] = m_table.write_default();
    root[JSON_CURRENT] = m_table.write_current();
    return root;
}
std::string FossilTable::to_cpp() const{
    std::string str;
    str += m_declaration;
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


