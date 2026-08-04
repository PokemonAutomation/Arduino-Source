/*  Static Globals
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <set>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Cryptography/SHA256.h"
#include "StaticGlobals.h"

namespace PokemonAutomation{



const std::set<std::string> TOKENS{
//    "f6538243092d8a3b9959bca988f054e1670f57c7246df2cbba25c4df3fe7a4e7",
    "2d04af67f6520e3550842d7eeb292868c6d0d4809b607f5a454712023d8815e1",
    "475d0a0a305a02cbf8b602bd47c3b275dccd5ac19fbe480729804a8e4e360b71",
    "6643d9fe87b3e54dc75dfac8ac22f0cc8bd17f6a8a786debf5fc4c517ee65469",
    "8e48e38e49bffc8462ada9d2d9d850d5b3b5c9529d20978c09bc548bc9a614a4",
    "7694adee4419d62c6a923c4efc9e7b41def7b96bb84ea882701b0bf2e8c13bee",
    "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08", //  jw's token.
    "e8d168bc482e96553ea9f9ecaea5a817474dbccc2a6a228a6bde67f2b2aa2889", //  James' token.
    "7555b7c63481cad42306718c67e7f9def5bfd1da8f6cd299ccd3d7dc95f307ae", //  Kuro's token.
    "3d475b46d121fc24559d100de2426feaa53cd6578aac2817c4857a610ccde2dd", //  kichi's token.
    "9b41db8175b5f248a78e738c7bd63a36e33b57953cb4e80ccdd13c2a7e892eec", //  Dalton's token.
};




void StaticGlobals::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }

    //  Naughty mode.
    obj->read_boolean(NAUGHTY_MODE, "NAUGHTY_MODE");

    //  Developer mode stuff.
    const std::string* dev_token = obj->get_string("DEVELOPER_TOKEN");
    if (dev_token){
        SHA256 hash;
        hash.push(dev_token->c_str(), dev_token->size());
        hash.finish();
        hash.get_hash_hex();
        DEVELOPER_MODE = TOKENS.find(hash.get_hash_hex()) != TOKENS.end();
    }

    const JsonObject* debug_obj = obj->get_object("DEBUG");
    if (debug_obj){
        debug_obj->read_boolean(COLOR_CHECK, "COLOR_CHECK");
        debug_obj->read_boolean(IMAGE_TEMPLATE_MATCHING, "IMAGE_TEMPLATE_MATCHING");
        debug_obj->read_boolean(IMAGE_DICTIONARY_MATCHING, "IMAGE_DICTIONARY_MATCHING");
        debug_obj->read_integer(BOX_SYSTEM_CELL_ROW, "BOX_SYSTEM_CELL_ROW");
        debug_obj->read_integer(BOX_SYSTEM_CELL_COL, "BOX_SYSTEM_CELL_COL");
        debug_obj->read_boolean(GENERATE_TEST_GOLDEN_FILES, "GENERATE_TEST_GOLDEN_FILES");
    }
}
JsonValue StaticGlobals::to_json_debug() const{
    JsonObject debug_obj;
    debug_obj["COLOR_CHECK"] = COLOR_CHECK;
    debug_obj["IMAGE_TEMPLATE_MATCHING"] = IMAGE_TEMPLATE_MATCHING;
    debug_obj["IMAGE_DICTIONARY_MATCHING"] = IMAGE_DICTIONARY_MATCHING;
    debug_obj["BOX_SYSTEM_CELL_ROW"] = BOX_SYSTEM_CELL_ROW;
    debug_obj["BOX_SYSTEM_CELL_COL"] = BOX_SYSTEM_CELL_COL;
    debug_obj["GENERATE_TEST_GOLDEN_FILES"] = GENERATE_TEST_GOLDEN_FILES;
    return debug_obj;
}



StaticGlobals STATIC_GLOBALS;





}
