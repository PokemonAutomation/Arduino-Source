/*  String Normalization
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include <QString>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Qt/StringToolsQt.h"
#include "CommonFramework/Globals.h"
#include "OCR_StringNormalization.h"

namespace PokemonAutomation{
namespace OCR{


std::u32string normalize_utf32(const std::string& text){
    QString qstr = QString::fromStdString(text);
    qstr = QString::fromStdU32String(run_character_reductions(qstr.toStdU32String()));
    qstr = qstr.normalized(QString::NormalizationForm_KD);
    std::u32string u32 = remove_non_alphanumeric(qstr.toStdU32String());
    u32 = run_character_reductions(u32);
    to_lower(u32);
    return u32;
}



std::u32string remove_non_alphanumeric(const std::u32string& text){
    std::u32string str;
    for (char32_t ch : text){
        if (QChar::isLetterOrNumber(ch)){
            str += ch;
            continue;
        }
    }
    return str;
}


std::map<char32_t, std::u32string> make_substitution_map32(){
    std::string path = RESOURCE_PATH() + "Tesseract/CharacterReductions.json";
    JsonValue json = load_json_file(path);
    JsonObject& obj = json.to_object_throw(path);

    std::map<char32_t, std::u32string> map;
    for (auto& item : obj){
        const std::string& target = item.first;
        std::string& sources = item.second.to_string_throw(path);
        for (char32_t ch : to_utf32(sources)){
            auto iter = map.find(ch);
            if (iter != map.end()){
                throw FileException(
                    nullptr, PA_CURRENT_FUNCTION,
                    to_utf8(to_utf32("Duplicate character reduction: ") + ch),
                    std::move(path)
                );
            }
            map[ch] = to_utf32(target);
        }
    }
    return map;
}

// Load character substitution rules from RESOURCE_PATH()/Tesseract/CharacterReductions.json
// and build a lookup map. The JSON file maps target characters to source characters that
// should be replaced, e.g. {"o": "0О"} means '0' and 'О' (Cyrillic O) should both map to 'o'.
// This helps normalize OCR output where visually similar characters are often misrecognized.
// Throws FileException if duplicate character mappings are found.
// Return a map of raw unicode character -> unicode string to become to.
// A map of {'0': 'o', 'O': 'o'} means for '0' and 'O' in the raw input text will be converted
// to 'o'.
const std::map<char32_t, std::u32string>& SUBSTITUTION_MAP32(){
    static std::map<char32_t, std::u32string> map = make_substitution_map32();
    return map;
}


std::u32string run_character_reductions(const std::u32string& text){
    // raw unicode character from text -> disambiguated unicode string
    const std::map<char32_t, std::u32string>& map = SUBSTITUTION_MAP32();
    std::u32string str;
    for (char32_t ch : text){
        auto iter = map.find(ch);
        if (iter == map.end()){
            str += ch;
        }else{
            str += iter->second;
        }
    }
    return str;
}







}
}

