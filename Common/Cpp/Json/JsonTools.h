/*  JSON Tools
 *
 *  From: https://github.com/PokemonAutomation/
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
// Load file from `filename` into a string
// If unable to open the file, FileException is thrown
std::string file_to_string(const std::string& filename);
// Load file from `filename` into a string. Return true if the file can be read successfully 
bool file_to_string(const std::string& filename, std::string& content);

JsonValue from_nlohmann(const nlohmann::json& json);
nlohmann::json to_nlohmann(const JsonValue& json);

JsonValue from_QJson(const QJsonValue& json);
QJsonValue to_QJson(const JsonValue& json);


}
#endif
