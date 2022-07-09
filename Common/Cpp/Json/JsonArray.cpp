/*  JSON Array
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "JsonArray.h"
#include "JsonTools.h"

namespace PokemonAutomation{



std::string JsonArray::dump(int indent) const{
    nlohmann::json::array_t ret;
    for (const auto& item : *this){
        ret.emplace_back(to_nlohmann(item));
    }
    return nlohmann::json(ret).dump(indent);
}
void JsonArray::dump(const std::string& filename, int indent) const{
    string_to_file(filename, dump(indent));
}



}
