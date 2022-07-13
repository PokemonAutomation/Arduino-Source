/*  String Normalization
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Qt/StringToolsQt.h"
#include "CommonFramework/Globals.h"
#include "OCR_StringNormalization.h"

namespace PokemonAutomation{
namespace OCR{



#if 0
QString normalize(QString text){
    text = text.normalized(QString::NormalizationForm_KD);
    text = remove_non_alphanumeric(text);
    text = run_character_reductions(text);
    text = text.toLower();
    return text;
}
#endif
std::u32string normalize_utf32(const std::string& text){
    QString qstr = QString::fromStdString(text);
    qstr = qstr.normalized(QString::NormalizationForm_KD);
    std::u32string u32 = remove_non_alphanumeric(qstr.toStdU32String());
    u32 = run_character_reductions(u32);
    to_lower(u32);
    return u32;
}




#if 0
QString remove_non_alphanumeric(const QString& text){
    QString str;
    for (QChar ch : text){
        if (ch.isLetterOrNumber()){
            str += ch;
            continue;
        }
#if 0
        if (ch == QChar(0x3099)){   //  Japanese dakuten.
            str += ch;
            continue;
        }
        if (ch == QChar(0x309A)){   //  Japanese handakuten.
            str += ch;
            continue;
        }
#endif
    }
    return str;
}
#endif
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



#if 0
QString remove_white_space(const QString& text){
    QString str;
    for (QChar ch : text){
        if (!ch.isSpace()){
            str += ch;
        }
    }
    return str;
}
bool strip_leading_trailing_non_alphanumeric(QString& text){
    bool changed = false;
    int s = 0;
    int e = text.size();
    while (s < e && !text[s].isLetterOrNumber()){
        s++;
        changed = true;
    }
    while (s < e && !text[e - 1].isLetterOrNumber()){
        e--;
        changed = true;
    }
    if (changed){
        text = QString(text.data() + s, e - s);
    }
    return changed;
}
#endif



#if 0
std::map<QChar, QString> make_substitution_map(){
    std::string path = RESOURCE_PATH() + "Tesseract/CharacterReductions.json";
    JsonValue json = load_json_file(path);
    JsonObject& obj = json.get_object_throw(path);

    std::map<QChar, QString> map;
    for (auto& item : obj){
        const std::string& target = item.first;
        std::string& sources = item.second.get_string_throw(path);
        for (QChar ch : QString::fromStdString(sources)){
            auto iter = map.find(ch);
            if (iter != map.end()){
                throw FileException(
                    nullptr, PA_CURRENT_FUNCTION,
                    (QString("Duplicate character reduction: ") + ch).toStdString(),
                    std::move(path)
                );
            }
            map[ch] = QString::fromStdString(target);
        }
    }
    return map;
}
const std::map<QChar, QString>& SUBSTITUTION_MAP(){
    static std::map<QChar, QString> map = make_substitution_map();
    return map;
}
QString run_character_reductions(const QString& text){
    const std::map<QChar, QString>& map = SUBSTITUTION_MAP();

    QString str;
    for (QChar ch : text){
        auto iter = map.find(ch);
        if (iter == map.end()){
            str += ch;
        }else{
            str += iter->second;
        }
    }
    return str;
}
#endif

std::map<char32_t, std::u32string> make_substitution_map32(){
    std::string path = RESOURCE_PATH() + "Tesseract/CharacterReductions.json";
    JsonValue json = load_json_file(path);
    JsonObject& obj = json.get_object_throw(path);

    std::map<char32_t, std::u32string> map;
    for (auto& item : obj){
        const std::string& target = item.first;
        std::string& sources = item.second.get_string_throw(path);
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
const std::map<char32_t, std::u32string>& SUBSTITUTION_MAP32(){
    static std::map<char32_t, std::u32string> map = make_substitution_map32();
    return map;
}
std::u32string run_character_reductions(const std::u32string& text){
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

