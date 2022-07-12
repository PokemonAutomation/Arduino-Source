/*  Configurable Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include "Common/Cpp/Json/JsonObject.h"
#include "Tools/Tools.h"
#include "SingleStatementOption.h"

namespace PokemonAutomation{

const std::string SingleStatementOption::JSON_DECLARATION  = "02-Declaration";
const std::string SingleStatementOption::JSON_DEFAULT      = "98-Default";
const std::string SingleStatementOption::JSON_CURRENT      = "99-Current";

SingleStatementOption::SingleStatementOption(const JsonObject& obj)
    : ConfigItem(obj)
    , m_declaration(obj.get_string_throw(JSON_DECLARATION))
{}

JsonObject SingleStatementOption::to_json() const{
    JsonObject root = ConfigItem::to_json();
    root[JSON_DECLARATION] = m_declaration;
    return root;
}



}







