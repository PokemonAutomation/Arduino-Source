/*  JSON Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Common_Json_JsonTools_H
#define PokemonAutomation_Common_Json_JsonTools_H

#include <vector>
#include "3rdParty/nlohmann/json.hpp"
#include "JsonValue.h"

class QJsonValue;

namespace PokemonAutomation{


void string_to_file(const std::string& filename, const std::string& str);
std::string file_to_string(const std::string& filename);

JsonValue from_nlohmann(const nlohmann::json& json);
nlohmann::json to_nlohmann(const JsonValue& json);

JsonValue from_QJson(const QJsonValue& json);
QJsonValue to_QJson(const JsonValue& json);


}
#endif
