/*  Multi-Host Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonArray>
#include <QVBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QLineEdit>
#include <QScrollBar>
#include "Common/Qt/StringException.h"
#include "Common/Qt/QtJsonTools.h"
#include "Common/Qt/ExpressionEvaluator.h"
#include "Tools/Tools.h"
#include "MultiHostTable.h"

//#include <iostream>
//using namespace std;

namespace PokemonAutomation{


const QString MultiHostTable::OPTION_TYPE = "MultiHostTable";

int MultiHostTable_init = register_option(
    MultiHostTable::OPTION_TYPE,
        [](const QJsonObject& obj){
        return std::unique_ptr<ConfigItem>(
            new MultiHostTable(obj)
        );
    }
);


MultiHostTable::MultiHostTable(const QJsonObject& obj)
    : SingleStatementOption(obj)
    , MultiHostTableOption(SingleStatementOption::m_label)
{
    load_default(json_get_array_throw(obj, JSON_DEFAULT));
    load_current(json_get_array_throw(obj, JSON_CURRENT));
}
bool MultiHostTable::is_valid() const{
    return MultiHostTableOption::is_valid();
}
void MultiHostTable::restore_defaults(){
    MultiHostTableOption::restore_defaults();
}
QJsonObject MultiHostTable::to_json() const{
    QJsonObject root = SingleStatementOption::to_json();
    root.insert(JSON_DEFAULT, write_default());
    root.insert(JSON_CURRENT, write_current());
    return root;
}
std::string MultiHostTable::to_cpp() const{
    std::string str;
    str += m_declaration.toUtf8().data();
    str += " = {\r\n";
    for (const auto& item : value()){
        str += "    {\r\n";
        str += std::string("        .game_slot        = ") + std::to_string(item.game_slot) + ",\r\n";
        str += std::string("        .user_slot        = ") + std::to_string(item.user_slot) + ",\r\n";
        str += std::string("        .skips            = ") + std::to_string(item.skips) + ",\r\n";
        str += std::string("        .backup_save      = ") + (item.backup_save ? "true" : "false") + ",\r\n";
        str += std::string("        .always_catchable = ") + (item.always_catchable ? "true" : "false") + ",\r\n";
        str += std::string("        .accept_FRs       = ") + (item.accept_FRs ? "true" : "false") + ",\r\n";
        str += std::string("        .move_slot        = ") + std::to_string(item.move_slot) + ",\r\n";
        str += std::string("        .dynamax          = ") + (item.dynamax ? "true" : "false") + ",\r\n";
        str += std::string("        .post_raid_delay  = ") + item.post_raid_delay.toUtf8().data() + ",\r\n";
        str += "    },\r\n";
    }
    str += "    {},\r\n";
    str += "};\r\n";
    return str;
}
QWidget* MultiHostTable::make_ui(QWidget& parent){
    return new MultiHostTableUI(parent, *this);
}


MultiHostTableUI::MultiHostTableUI(QWidget& parent, MultiHostTable& value)
    : MultiHostTableOptionUI(parent, value)
{}



}


