/*  Qt JSON Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include "StringException.h"
#include "QtJsonTools.h"

namespace PokemonAutomation{


QJsonDocument read_json_file(const QString& path){
    QFile file(path);
    if (!file.open(QFile::ReadOnly)){
        throw StringException("Unable to open file: " + path);
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
        throw StringException("Unable to create settings file.\r\n" + path);
    }
    if (file.write(json_out.c_str(), json_out.size()) != (int)json_out.size()){
        throw StringException("Unable to write settings file.\r\n" + path);
    }
    file.close();
}



int json_cast_int(const QJsonValue& value){
    if (!value.isDouble()){
        throw StringException("Config Error - Expected a number.");
    }
    return value.toInt();
}
QString json_cast_string(const QJsonValue& value){
    if (!value.isString()){
        throw StringException("Config Error - Expected a string.");
    }
    return value.toString();
}
QJsonArray json_cast_array(const QJsonValue& value){
    if (!value.isArray()){
        throw StringException("Config Error - Expected an array.");
    }
    return value.toArray();
}
QJsonObject json_cast_object(const QJsonValue& value){
    if (!value.isObject()){
        throw StringException("Config Error - Expected an object.");
    }
    return value.toObject();
}



QJsonValue json_get_value(const QJsonObject& obj, const QString& key){
    auto iter = obj.find(key);
    if (iter == obj.end()){
        throw StringException("Config Error - Key not found: " + key);
    }
    return *iter;
}
bool json_get_bool(const QJsonObject& obj, const QString& key){
    auto iter = obj.find(key);
    if (iter == obj.end()){
        throw StringException("Config Error - Key not found: " + key);
    }
    if (!iter->isBool()){
        throw StringException("Config Error - Expected a boolean: " + key);
    }
    return iter->toBool();
}
int json_get_int(const QJsonObject& obj, const QString& key){
    auto iter = obj.find(key);
    if (iter == obj.end()){
        throw StringException("Config Error - Key not found: " + key);
    }
    if (!iter->isDouble()){
        throw StringException("Config Error - Expected a number: " + key);
    }
    return iter->toInt();
}
double json_get_double(const QJsonObject& obj, const QString& key){
    auto iter = obj.find(key);
    if (iter == obj.end()){
        throw StringException("Config Error - Key not found: " + key);
    }
    if (!iter->isDouble()){
        throw StringException("Config Error - Expected a number: " + key);
    }
    return iter->toDouble();
}
QString json_get_string(const QJsonObject& obj, const QString& key){
    auto iter = obj.find(key);
    if (iter == obj.end()){
        throw StringException("Config Error - Key not found: " + key);
    }
    if (!iter->isString()){
        throw StringException("Config Error - Expected a string: " + key);
    }
    return iter->toString();
}
QJsonArray json_get_array(const QJsonObject& obj, const QString& key){
    auto iter = obj.find(key);
    if (iter == obj.end()){
        throw StringException("Config Error - Key not found: " + key);
    }
    if (!iter->isArray()){
        throw StringException("Config Error - Expected an array: " + key);
    }
    return iter->toArray();
}
QJsonObject json_get_object(const QJsonObject& obj, const QString& key){
    auto iter = obj.find(key);
    if (iter == obj.end()){
        throw StringException("Config Error - Key not found: " + key);
    }
    if (!iter->isObject()){
        throw StringException("Config Error - Expected an array: " + key);
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
QDate json_parse_date(const QJsonValue& value){
    QJsonArray array = json_cast_array(value);
    if (array.size() != 3){
        throw StringException("Config Error - Date should be 3 elements");
    }
    int year = json_cast_int(array[0]);
    int month = json_cast_int(array[1]);
    int day = json_cast_int(array[2]);
    QDate date(year, month, day);
    if (!date.isValid()){
        throw StringException("Config Error - Invalid Date");
    }
    if (!valid_switch_date(date)){
        throw StringException("Config Error - Invalid Switch Date");
    }
    return date;
}
QDate json_get_date(const QJsonObject& obj, const QString& key){
    return json_parse_date(json_get_value(obj, key));
}



}



