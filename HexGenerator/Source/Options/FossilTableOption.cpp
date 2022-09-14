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
#include "Common/Qt/Options/ConfigWidget.h"
#include "Tools/Tools.h"
#include "FossilTableOption.h"

namespace PokemonAutomation{
namespace HexGenerator{


const std::string FossilTable::OPTION_TYPE      = "FossilTable";


const std::vector<std::string> FossilTable::FOSSIL_LIST{
    "Dracozolt",
    "Arctozolt",
    "Dracovish",
    "Arctovish",
};


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
{
    m_table.load_json(obj.get_value_throw(JSON_CURRENT));
}
std::string FossilTable::check_validity() const{
    return m_table.check_validity();
}
void FossilTable::restore_defaults(){
    m_table.restore_defaults();
}
JsonObject FossilTable::to_json() const{
    JsonObject root = SingleStatementOption::to_json();
    root[JSON_CURRENT] = m_table.to_json();
    return root;
}
std::string FossilTable::to_cpp() const{
    std::string str;
    str += m_declaration;
    str += " = {\r\n";
    std::vector<std::unique_ptr<NintendoSwitch::PokemonSwSh::FossilGame>> list = m_table.copy_snapshot();
    for (size_t c = 0; c < list.size(); c++){
        const NintendoSwitch::PokemonSwSh::FossilGame& item = *list[c];
        str += "    {";
        str += std::to_string(item.game_slot.current_value());
        str += ", ";
        str += std::to_string(item.user_slot.current_value());
        str += ", ";
        str += FOSSIL_LIST[item.fossil.current_value()];
        str += ", ";
        str += std::to_string(item.revives);
        str += "},\r\n";
    }
    str += "    {},\r\n";
    str += "};\r\n";
    return str;
}
QWidget* FossilTable::make_ui(QWidget& parent){
    return &m_table.make_QtWidget(parent)->widget();
}







}
}


