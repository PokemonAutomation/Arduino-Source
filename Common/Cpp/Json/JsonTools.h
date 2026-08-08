/*  JSON Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Common_Json_JsonTools_H
#define PokemonAutomation_Common_Json_JsonTools_H

#include "3rdParty/nlohmann/json.hpp"
#include "JsonValue.h"

namespace PokemonAutomation{


JsonValue from_nlohmann(const nlohmann::json& json);
nlohmann::json to_nlohmann(const JsonValue& json);


}
#endif
