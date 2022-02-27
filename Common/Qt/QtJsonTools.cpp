/*  Qt JSON Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Exception.h"
#include "QtJsonTools.h"

namespace PokemonAutomation{


QJsonDocument read_json_file(const QString& path){
    QFile file(path);
    if (!file.open(QFile::ReadOnly)){
        throw FileException(nullptr, __PRETTY_FUNCTION__, "Unable to open file.", path.toStdString());
    }
//    auto data = file.readAll();
//    return QJsonDocument::fromJson(data);
    return QJsonDocument::fromJson(file.readAll());
}
void write_json_file(const QString& path, const QJsonDocument& json){
    std::string json_raw = json.toJson().toStdString();
    std::string json_out = "\xef\xbb\xbf";
    //  Convert to CRLF.
    char previous = 0;
    for (char ch : json_raw){
        if (ch == '\n' && previous != '\r'){
            json_out += '\r';
        }
        json_out += ch;
        previous = ch;
    }

    QFile file(path);
    if (!file.open(QFile::WriteOnly)){
        throw FileException(nullptr, __PRETTY_FUNCTION__, "Unable to create file.", path.toStdString());
    }
    if (file.write(json_out.c_str(), json_out.size()) != (int)json_out.size()){
        throw FileException(nullptr, __PRETTY_FUNCTION__, "Unable to write file.", path.toStdString());
    }
    file.close();
}



QJsonValue json_get_value_throw(const QJsonObject& obj, const QString& key){
    auto iter = obj.find(key);
    if (iter == obj.end()){
        throw ParseException("Config Error - Key not found: " + key.toStdString());
    }
    return *iter;
}
bool json_get_bool_throw(const QJsonObject& obj, const QString& key){
    auto iter = obj.find(key);
    if (iter == obj.end()){
        throw ParseException("Config Error - Key not found: " + key.toStdString());
    }
    if (!iter->isBool()){
        throw ParseException("Config Error - Expected a boolean: " + key.toStdString());
    }
    return iter->toBool();
}
int json_get_int_throw(const QJsonObject& obj, const QString& key){
    auto iter = obj.find(key);
    if (iter == obj.end()){
        throw ParseException("Config Error - Key not found: " + key.toStdString());
    }
    if (!iter->isDouble()){
        throw ParseException("Config Error - Expected a number: " + key.toStdString());
    }
    return iter->toInt();
}
double json_get_double_throw(const QJsonObject& obj, const QString& key){
    auto iter = obj.find(key);
    if (iter == obj.end()){
        throw ParseException("Config Error - Key not found: " + key.toStdString());
    }
    if (!iter->isDouble()){
        throw ParseException("Config Error - Expected a number: " + key.toStdString());
    }
    return iter->toDouble();
}
QString json_get_string_throw(const QJsonObject& obj, const QString& key){
    auto iter = obj.find(key);
    if (iter == obj.end()){
        throw ParseException("Config Error - Key not found: " + key.toStdString());
    }
    if (!iter->isString()){
        throw ParseException("Config Error - Expected a string: " + key.toStdString());
    }
    return iter->toString();
}
QJsonArray json_get_array_throw(const QJsonObject& obj, const QString& key){
    auto iter = obj.find(key);
    if (iter == obj.end()){
        throw ParseException("Config Error - Key not found: " + key.toStdString());
    }
    if (!iter->isArray()){
        throw ParseException("Config Error - Expected a array: " + key.toStdString());
    }
    return iter->toArray();
}
QJsonObject json_get_object_throw(const QJsonObject& obj, const QString& key){
    auto iter = obj.find(key);
    if (iter == obj.end()){
        throw ParseException("Config Error - Key not found: " + key.toStdString());
    }
    if (!iter->isObject()){
        throw ParseException("Config Error - Expected a object: " + key.toStdString());
    }
    return iter->toObject();
}


QString json_get_string_nothrow(const QJsonObject& obj, const QString& key){
    QString value;
    json_get_string(value, obj, key);
    return value;
}
QJsonValue json_get_value_nothrow(const QJsonObject& obj, const QString& key){
    auto iter = obj.find(key);
    if (iter == obj.end()){
        return QJsonValue();
    }
    return *iter;
}
QJsonArray json_get_array_nothrow(const QJsonObject& obj, const QString& key){
    auto iter = obj.find(key);
    if (iter == obj.end() || !iter->isArray()){
        return QJsonArray();
    }
    return iter->toArray();
}
QJsonObject json_get_object_nothrow(const QJsonObject& obj, const QString& key){
    auto iter = obj.find(key);
    if (iter == obj.end() || !iter->isObject()){
        return QJsonObject();
    }
    return iter->toObject();
}



bool valid_switch_date(const QDate& date){
    if (date < QDate(2000, 1, 1)){
        return false;
    }
    if (date > QDate(2060, 12, 31)){
        return false;
    }
    return true;
}
QJsonArray json_write_date(const QDate& date){
    QJsonArray array;
    array += date.year();
    array += date.month();
    array += date.day();
    return array;
}



}



