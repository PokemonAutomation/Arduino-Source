/*  Multi-Host Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QLineEdit>
#include <QScrollBar>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonTools.h"
#include "Common/Qt/ExpressionEvaluator.h"
#include "Common/Qt/Options/EditableTable/EditableTableBaseWidget.h"
#include "Tools/Tools.h"
#include "MultiHostTableOption.h"

//#include <iostream>
//using namespace std;

namespace PokemonAutomation{
namespace HexGenerator{


const std::string MultiHostTable::OPTION_TYPE = "MultiHostTable";

int MultiHostTable_init = register_option(
    MultiHostTable::OPTION_TYPE,
        [](const JsonObject& obj){
        return std::unique_ptr<ConfigItem>(
            new MultiHostTable(obj)
        );
    }
);


MultiHostTable::MultiHostTable(const JsonObject& obj)
    : SingleStatementOption(obj)
    , m_factory(false)
    , m_table(SingleStatementOption::m_label, m_factory)
{
    m_table.load_default(obj.get_value_throw(JSON_DEFAULT));
    m_table.load_current(obj.get_value_throw(JSON_CURRENT));
}
std::string MultiHostTable::check_validity() const{
    return m_table.check_validity();
}
void MultiHostTable::restore_defaults(){
    m_table.restore_defaults();
}
JsonObject MultiHostTable::to_json() const{
    JsonObject root = SingleStatementOption::to_json();
    root[JSON_DEFAULT] = m_table.write_default();
    root[JSON_CURRENT] = m_table.write_current();
    return root;
}
std::string MultiHostTable::to_cpp() const{
    std::string str;
    str += m_declaration;
    str += " = {\r\n";
    for (size_t c = 0; c < m_table.size(); c++){
        const NintendoSwitch::PokemonSwSh::MultiHostSlot& item = static_cast<const NintendoSwitch::PokemonSwSh::MultiHostSlot&>(m_table[c]);
        str += "    {\r\n";
        str += std::string("        .game_slot        = ") + std::to_string(item.game_slot) + ",\r\n";
        str += std::string("        .user_slot        = ") + std::to_string(item.user_slot) + ",\r\n";
        str += std::string("        .skips            = ") + std::to_string(item.skips) + ",\r\n";
        str += std::string("        .backup_save      = ") + (item.backup_save ? "true" : "false") + ",\r\n";
        str += std::string("        .always_catchable = ") + (item.always_catchable ? "true" : "false") + ",\r\n";
        str += std::string("        .accept_FRs       = ") + (item.accept_FRs ? "true" : "false") + ",\r\n";
        str += std::string("        .move_slot        = ") + std::to_string(item.move_slot) + ",\r\n";
        str += std::string("        .dynamax          = ") + (item.dynamax ? "true" : "false") + ",\r\n";
        str += std::string("        .post_raid_delay  = ") + item.post_raid_delay + ",\r\n";
        str += "    },\r\n";
    }
    str += "    {},\r\n";
    str += "};\r\n";
    return str;
}
QWidget* MultiHostTable::make_ui(QWidget& parent){
    return new EditableTableBaseWidget(parent, m_table);
}



}
}


