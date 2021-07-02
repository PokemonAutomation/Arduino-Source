/*  String Normalization
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include <QtGlobal>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/PersistentSettings.h"
#include "StringNormalization.h"

namespace PokemonAutomation{
namespace OCR{




QString normalize(QString text){
    text = text.normalized(QString::NormalizationForm_KD);
    text = remove_non_alphanumeric(text);
    text = run_character_reductions(text);
    text = text.toLower();
    return text;
}




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




std::map<QChar, QString> make_substitution_map(){
    QJsonObject obj = read_json_file(
        PERSISTENT_SETTINGS().resource_path + "Tesseract/CharacterReductions.json"
    ).object();

    std::map<QChar, QString> map;
    for (auto item = obj.begin(); item != obj.end(); ++item){
        QString target = item.key();
        QString sources = item.value().toString();
        for (QChar ch : sources){
            auto iter = map.find(ch);
            if (iter != map.end()){
                PA_THROW_StringException(QString("Duplicate character reduction: ") + ch);
            }
            map[ch] = target;
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








}
}

