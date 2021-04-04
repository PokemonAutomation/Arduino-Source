/*  Configurable Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include <QJsonObject>
#include "Common/Qt/QtJsonTools.h"
#include "Tools/Tools.h"
#include "SingleStatementOption.h"

namespace PokemonAutomation{

const QString SingleStatementOption::JSON_DECLARATION  = "02-Declaration";
const QString SingleStatementOption::JSON_DEFAULT      = "98-Default";
const QString SingleStatementOption::JSON_CURRENT      = "99-Current";

SingleStatementOption::SingleStatementOption(const QJsonObject& obj)
    : ConfigItem(obj)
    , m_declaration(json_get_string_throw(obj, JSON_DECLARATION))
{}

QJsonObject SingleStatementOption::to_json() const{
    QJsonObject root = ConfigItem::to_json();
    root.insert(JSON_DECLARATION, m_declaration);
    return root;
}



}







